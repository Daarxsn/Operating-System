.section .text

.global user_thread_enter
.type user_thread_enter, @function

/*
 * Enter Ring 3.
 *
 * RAX = user RIP
 * RDX = user RSP
 *
 * iretq expects:
 *
 *   SS
 *   RSP
 *   RFLAGS
 *   CS
 *   RIP
 */

user_thread_enter:

    cli

    /* User data selector: 0x30 | RPL3 = 0x33 */
    pushq $0x33

    /* User stack */
    pushq %rdx

    /* IF=1, reserved bit 1 set */
    pushq $0x202

    /* User code selector: 0x28 | RPL3 = 0x2B */
    pushq $0x2B

    /* User entry point */
    pushq %rax

    iretq

.section .note.GNU-stack,"",@progbits
