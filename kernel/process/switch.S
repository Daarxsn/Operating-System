.section .text

.global context_switch

.type context_switch, @function

context_switch:

    /*
     * RDI = current context
     * RSI = next context
     */

    /*
     * Save current stack pointer.
     */
    movq %rsp, 0(%rdi)

    /*
     * Save callee-saved registers.
     */
    movq %rbp, 8(%rdi)
    movq %rbx, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

    /*
     * Save return address as RIP.
     */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)

    /*
     * Save current RFLAGS.
     */
    pushfq
    popq %rax
    movq %rax, 64(%rdi)

    /*
     * Load next context.
     */
    movq 0(%rsi), %rsp

    movq 8(%rsi), %rbp
    movq 16(%rsi), %rbx
    movq 24(%rsi), %r12
    movq 32(%rsi), %r13
    movq 40(%rsi), %r14
    movq 48(%rsi), %r15

    /*
     * Restore flags.
     */
    pushq 64(%rsi)
    popfq

    /*
     * Jump into next thread.
     */
    jmp *56(%rsi)

.size context_switch, .-context_switch

.section .note.GNU-stack,"",@progbits
