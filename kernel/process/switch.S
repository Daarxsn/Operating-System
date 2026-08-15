.section .text

.global context_switch
.type context_switch, @function

/*
 * XyrisOS x86-64 context switch.
 *
 * Important interrupt-preemption detail:
 *
 * scheduler_schedule() may be called from irq_dispatch(), i.e. while
 * the CPU is inside an interrupt gate.  The CPU clears IF on entry to
 * that gate.  The old implementation restored RFLAGS with POPFQ before
 * leaving context_switch(), which could re-enable interrupts while the
 * new thread was still executing inside the old interrupt handler.
 *
 * We therefore save RFLAGS for the context, but do NOT restore the whole
 * flags word with POPFQ.  We restore only the interrupt-enable state:
 *   - saved IF = 0: leave interrupts disabled while resuming the old
 *     interrupt handler; its eventual IRETQ restores the interrupted
 *     thread's original RFLAGS.
 *   - saved IF = 1: enable interrupts before entering/resuming a normal
 *     thread context.
 *
 * This lets the existing lightweight context format work for both
 * cooperative and timer-driven switches.
 */

context_switch:
    /* RDI = current context, RSI = next context */

    /* Save stack pointer and callee-saved registers. */
    movq %rsp, 0(%rdi)
    movq %rbp, 8(%rdi)
    movq %rbx, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

    /* Save the return address of context_switch(). */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)

    /* Save current RFLAGS, including IF. */
    pushfq
    popq %rax
    movq %rax, 64(%rdi)

    /* Load next context. */
    movq 0(%rsi), %rsp
    movq 8(%rsi), %rbp
    movq 16(%rsi), %rbx
    movq 24(%rsi), %r12
    movq 32(%rsi), %r13
    movq 40(%rsi), %r14
    movq 48(%rsi), %r15

    /*
     * Do not POPFQ here.  In particular, never enable IF while the
     * resumed context is still below an interrupt handler's stack frame.
     * Only reproduce the IF state required by the destination context.
     */
    testq $0x200, 64(%rsi)
    jz 1f
    sti
1:
    /* Enter the destination context. */
    jmp *56(%rsi)

.size context_switch, .-context_switch

.section .note.GNU-stack,"",@progbits
