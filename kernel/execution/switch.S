.section .text

.global switch_context
.global switch_to_context

/*
 * ------------------------------------------------------------
 * switch_context
 * ------------------------------------------------------------
 *
 * rdi = old context
 * rsi = next context
 *
 * Saves/restores the callee-saved registers required by the
 * System V AMD64 ABI, plus RSP and RIP.
 */

switch_context:

    /* Save callee-saved registers */
    movq %r15, 0(%rdi)
    movq %r14, 8(%rdi)
    movq %r13, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %rbx, 32(%rdi)
    movq %rbp, 40(%rdi)

    /* Save stack pointer */
    movq %rsp, 48(%rdi)

    /*
     * Save return address of switch_context().
     */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)

    /* Restore callee-saved registers */
    movq 0(%rsi), %r15
    movq 8(%rsi), %r14
    movq 16(%rsi), %r13
    movq 24(%rsi), %r12
    movq 32(%rsi), %rbx
    movq 40(%rsi), %rbp

    /* Restore stack pointer */
    movq 48(%rsi), %rsp

    /* Restore instruction pointer */
    movq 56(%rsi), %rax
    jmp *%rax


/*
 * ------------------------------------------------------------
 * switch_to_context
 * ------------------------------------------------------------
 *
 * rdi = next context
 *
 * Used when there is no currently running thread.
 *
 * Does NOT save an old context.
 */

switch_to_context:

    /* Restore callee-saved registers */
    movq 0(%rdi), %r15
    movq 8(%rdi), %r14
    movq 16(%rdi), %r13
    movq 24(%rdi), %r12
    movq 32(%rdi), %rbx
    movq 40(%rdi), %rbp

    /* Restore stack pointer */
    movq 48(%rdi), %rsp

    /* Jump directly to the new thread */
    movq 56(%rdi), %rax
    jmp *%rax


/*
 * Prevent executable stack warning.
 */

.section .note.GNU-stack,"",@progbits
