.section .text

.global user_thread_enter
.extern user_thread_trampoline

/*
 * Enter Ring 3 using the current thread's saved user_entry
 * and user_stack.
 *
 * The C trampoline supplies:
 *
 *   RAX = user entry
 *   RDX = user stack
 *
 * We construct the exact stack expected by IRETQ:
 *
 *   SS
 *   RSP
 *   RFLAGS
 *   CS
 *   RIP
 */

user_thread_enter:

    cli

    /* User SS = 0x33 */
    pushq $0x33

    /* User RSP */
    pushq %rdx

    /* RFLAGS: IF enabled */
    pushq $0x202

    /* User CS = 0x2B */
    pushq $0x2B

    /* User RIP */
    pushq %rax

    iretq