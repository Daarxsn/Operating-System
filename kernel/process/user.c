#include "user.h"

#include "../memory/pmm.h"

#define USER_STACK_TOP  0x00007FFFFFF00000ULL
#define USER_STACK_SIZE (16 * 4096ULL)

int user_prepare(
    user_process_t* process,
    uint64_t entry)
{
    if (!process ||
        entry == 0 ||
        entry >= 0x0000800000000000ULL)
    {
        return -1;
    }

    *process = (user_process_t){0};

    /*
     * ----------------------------------------------------
     * Create Address Space
     * ----------------------------------------------------
     */

    process->address_space =
        vmm_create_space();

    if (!process->address_space)
    {
        return -1;
    }

    /*
     * ----------------------------------------------------
     * Configure User Stack
     * ----------------------------------------------------
     */

    process->stack_base =
        USER_STACK_TOP - USER_STACK_SIZE;

    process->stack_size =
        USER_STACK_SIZE;

    /*
     * ----------------------------------------------------
     * Allocate and Map User Stack
     * ----------------------------------------------------
     *
     * The physical pages allocated here are owned by
     * this user process.
     *
     * vmm_unmap_page() removes the virtual mapping but
     * does NOT free the physical frame.
     *
     * Therefore user_destroy() is responsible for
     * releasing these physical pages.
     */

    for (uint64_t va = process->stack_base;
         va < USER_STACK_TOP;
         va += 4096ULL)
    {
        phys_addr_t page =
            pmm_alloc_page();

        if (page == 0)
        {
            user_destroy(process);
            return -1;
        }

        if (!vmm_map_page(
                process->address_space,
                va,
                page,
                VMM_USER |
                VMM_WRITABLE |
                VMM_NX))
        {
            /*
             * Mapping failed.
             *
             * Ownership of this physical page was never
             * transferred to the address space.
             */
            pmm_free_page(page);

            user_destroy(process);
            return -1;
        }
    }

    /*
     * ----------------------------------------------------
     * Entry Point
     * ----------------------------------------------------
     */

    process->entry =
        entry;

    /*
     * Keep the stack 16-byte aligned according to the
     * normal x86-64 ABI alignment convention.
     */
    process->stack =
        USER_STACK_TOP - 16;

    return 0;
}


/*
 * --------------------------------------------------------
 * Destroy User Process
 * --------------------------------------------------------
 *
 * Releases:
 *
 *   1. Physical pages belonging to the user stack
 *   2. User virtual mappings
 *   3. User page-table hierarchy
 *
 * vmm_destroy_space() owns only the page-table pages.
 * The user process owns the physical stack frames.
 * --------------------------------------------------------
 */

void user_destroy(
    user_process_t* process)
{
    if (!process)
    {
        return;
    }

    if (process->address_space != NULL)
    {
        /*
         * ------------------------------------------------
         * Release User Stack Physical Pages
         * ------------------------------------------------
         */

        for (uint64_t va = process->stack_base;
             va < process->stack_base + process->stack_size;
             va += 4096ULL)
        {
            phys_addr_t page =
                vmm_translate(
                    process->address_space,
                    va
                );

            if (page != 0)
            {
                /*
                 * First remove the virtual mapping.
                 */
                if (vmm_unmap_page(
        process->address_space,
        va))
{
    pmm_free_page(
        page & ~(phys_addr_t)(4096ULL - 1)
    );
}
            }
        }

        /*
         * ------------------------------------------------
         * Destroy Address Space
         * ------------------------------------------------
         *
         * At this point all user stack mappings have been
         * removed and their physical pages released.
         *
         * vmm_destroy_space() now releases the remaining
         * page-table hierarchy.
         */

        vmm_destroy_space(
            process->address_space
        );
    }

    /*
     * Clear the process structure.
     */
    *process =
        (user_process_t){0};
}


/*
 * --------------------------------------------------------
 * Enter User Mode
 * --------------------------------------------------------
 */

void user_enter(
    user_process_t* process)
{
    if (!process ||
        !process->address_space ||
        !process->entry ||
        !process->stack)
    {
        return;
    }

    /*
     * Never enter an unmapped/non-user/non-executable entry point.
     * user_prepare() creates the stack, while the ELF loader is
     * responsible for mapping the program image.
     */
    uint64_t entry_flags =
        vmm_get_page_flags(
            process->address_space,
            process->entry & ~(uint64_t)(PAGE_SIZE - 1)
        );

    if ((entry_flags & VMM_PRESENT) == 0 ||
        (entry_flags & VMM_USER) == 0 ||
        (entry_flags & VMM_NX) != 0)
    {
        return;
    }

    /*
     * The initial user stack must be a writable user mapping.
     */
    uint64_t stack_flags =
        vmm_get_page_flags(
            process->address_space,
            process->stack & ~(uint64_t)(PAGE_SIZE - 1)
        );

    if ((stack_flags & VMM_PRESENT) == 0 ||
        (stack_flags & VMM_USER) == 0 ||
        (stack_flags & VMM_WRITABLE) == 0)
    {
        return;
    }

    /*
     * Switch to the user's address space.
     */
    vmm_switch_space(
        process->address_space
    );

    /*
     * User code/data descriptors are installed by gdt_init().
     *
     * GDT:
     *
     *   0x2B = user code
     *   0x33 = user data
     */

    __asm__ volatile(
        "cli\n\t"

        "pushq $0x33\n\t"          /* user SS */
        "pushq %[stack]\n\t"

        "pushfq\n\t"
        "orq $0x200, (%%rsp)\n\t"  /* IF */

        "pushq $0x2B\n\t"          /* user CS */
        "pushq %[entry]\n\t"

        "iretq\n\t"

        :
        : [stack] "r" (process->stack),
          [entry] "r" (process->entry)
        : "memory"
    );

    __builtin_unreachable();
}