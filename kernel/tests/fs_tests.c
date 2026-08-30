#include "fs_tests.h"

#include "../boot/boot.h"
#include "../fs/ramfs.h"
#include "../fs/file.h"
#include "../syscall/syscall.h"
#include "../loader/elf.h"
#include "../process/user.h"
#include "../memory/pmm.h"
#include "../lib/string.h"
#include "../debug/print.h"
#include "../debug/hex.h"

static void test_ok(const char *name, int condition)
{
    if (condition)
        boot_step_ok(name);
    else
        boot_step_fail(name);
}

void run_filesystem_tests(void)
{
    /* -------------------------------------------------
       RAMFS creation and path lookup
    ------------------------------------------------- */

    ramfs_node_t *dir =
        ramfs_create_directory("/etc");

    test_ok(
        "RAMFS Test: Create Directory",
        dir != NULL
    );

    ramfs_node_t *file =
        ramfs_create_file("/etc/xyris.txt");

    test_ok(
        "RAMFS Test: Create File",
        file != NULL
    );

    vnode_t *looked_up =
        vfs_lookup("/etc/xyris.txt");

    test_ok(
        "VFS Test: Path Lookup",
        looked_up != NULL &&
        looked_up->type == VFS_NODE_FILE
    );


    /* -------------------------------------------------
       File operations
    ------------------------------------------------- */

    int fd =
        open("/etc/xyris.txt");

    test_ok(
        "File Test: Open",
        fd >= 0
    );

    const char message[] =
        "XyrisOS Phase 3";

    int written =
        fd >= 0
            ? write(
                fd,
                message,
                sizeof(message) - 1
              )
            : -1;

    test_ok(
        "File Test: Write",
        written == (int)(sizeof(message) - 1)
    );

    if (fd >= 0)
        close(fd);


    fd =
        open("/etc/xyris.txt");

    char buffer[32];

    memset(
        buffer,
        0,
        sizeof(buffer)
    );

    int read_count =
        fd >= 0
            ? read(
                fd,
                buffer,
                sizeof(buffer) - 1
              )
            : -1;

    test_ok(
        "File Test: Read",
        read_count == written &&
        memcmp(
            buffer,
            message,
            sizeof(message) - 1
        ) == 0
    );

    if (fd >= 0)
    {
        test_ok(
            "File Test: Close",
            close(fd) == 0
        );
    }
    else
    {
        test_ok(
            "File Test: Close",
            0
        );
    }


    /* -------------------------------------------------
       System-call dispatch
    ------------------------------------------------- */

    fd =
        (int)syscall_dispatch(
            XYRIS_SYS_OPEN,
            (uint64_t)"/etc/xyris.txt",
            0,
            0,
            0
        );

    test_ok(
        "Syscall Test: Open",
        fd >= 0
    );

    memset(
        buffer,
        0,
        sizeof(buffer)
    );

    read_count =
        fd >= 0
            ? (int)syscall_dispatch(
                XYRIS_SYS_READ,
                (uint64_t)fd,
                (uint64_t)buffer,
                sizeof(buffer) - 1,
                0
              )
            : -1;

    test_ok(
        "Syscall Test: Read",
        read_count == written &&
        memcmp(
            buffer,
            message,
            sizeof(message) - 1
        ) == 0
    );

    if (fd >= 0)
    {
        uint64_t result =
            syscall_dispatch(
                XYRIS_SYS_CLOSE,
                (uint64_t)fd,
                0,
                0,
                0
            );

        test_ok(
            "Syscall Test: Close",
            (int64_t)result == 0
        );
    }
    else
    {
        test_ok(
            "Syscall Test: Close",
            0
        );
    }


    /* -------------------------------------------------
       ELF validation
    ------------------------------------------------- */

    Elf64_Ehdr header;

    memset(
        &header,
        0,
        sizeof(header)
    );

    header.e_ident[0] = 0x7F;
    header.e_ident[1] = 'E';
    header.e_ident[2] = 'L';
    header.e_ident[3] = 'F';
    header.e_ident[4] = ELFCLASS64;
    header.e_ident[5] = ELFDATA2LSB;

    header.e_version =
        1;

    header.e_machine =
        EM_X86_64;

    header.e_ehsize =
        sizeof(Elf64_Ehdr);

    header.e_phentsize =
        sizeof(Elf64_Phdr);

    header.e_phnum =
        1;

    header.e_entry =
        0x400000;

    test_ok(
        "ELF Test: Header Validation",
        elf_validate(&header)
    );

    test_ok(
        "ELF Test: Entry Point",
        elf_get_entry(&header) == 0x400000
    );


    /* -------------------------------------------------
       User Address Space Test
    ------------------------------------------------- */

    pmm_stats_t before =
        pmm_get_stats();

    debug_print(
        "USER BEFORE FREE = "
    );

    debug_print_hex64(
        (uint64_t)before.free_pages
    );

    debug_print("\n");


    debug_print(
        "USER BEFORE USED = "
    );

    debug_print_hex64(
        (uint64_t)before.used_pages
    );

    debug_print("\n");


    user_process_t user;

    int prepared =
        user_prepare(
            &user,
            0x400000
        ) == 0;

    test_ok(
        "User Test: Address Space Preparation",
        prepared
    );


    if (prepared)
    {
        user_destroy(
            &user
        );
    }


    pmm_stats_t after =
        pmm_get_stats();


    debug_print(
        "USER AFTER FREE  = "
    );

    debug_print_hex64(
        (uint64_t)after.free_pages
    );

    debug_print("\n");


    debug_print(
        "USER AFTER USED  = "
    );

    debug_print_hex64(
        (uint64_t)after.used_pages
    );

    debug_print("\n");


    test_ok(
        "User Test: Address Space Cleanup",
        before.free_pages == after.free_pages &&
        before.used_pages == after.used_pages
    );
}
