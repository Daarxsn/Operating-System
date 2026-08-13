.section .text

.extern isr_handler

/* -------------------------------------------------- */
/* Save Registers                                     */
/* -------------------------------------------------- */

.macro PUSH_REGS
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx

    pushq %rbp

    pushq %rdi
    pushq %rsi

    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

/* -------------------------------------------------- */

.macro POP_REGS
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8

    popq %rsi
    popq %rdi

    popq %rbp

    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
.endm

/* -------------------------------------------------- */

isr_common:

    PUSH_REGS

    movq %rsp, %rdi
    call isr_handler

    POP_REGS

    addq $16, %rsp

    iretq

/* -------------------------------------------------- */
/* Exceptions WITHOUT error code                      */
/* -------------------------------------------------- */

.macro ISR_NOERR num
.global isr\num

isr\num:
    pushq $0
    pushq $\num
    jmp isr_common
.endm

/* -------------------------------------------------- */
/* Exceptions WITH error code                         */
/* -------------------------------------------------- */

.macro ISR_ERR num
.global isr\num

isr\num:
    pushq $\num
    jmp isr_common
.endm

/* -------------------------------------------------- */
/* Generate CPU Exceptions                            */
/* -------------------------------------------------- */

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

ISR_ERR   8

ISR_NOERR 9

ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14

ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17

ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21

ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28

ISR_ERR   29
ISR_ERR   30

ISR_NOERR 31

/* -------------------------------------------------- */
/* Hardware IRQs (Vectors 32-47)                      */
/* -------------------------------------------------- */

.macro IRQ num vector
.global irq\num

irq\num:
    pushq $0
    pushq $\vector
    jmp isr_common
.endm

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39

IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* -------------------------------------------------- */
/* System Call Vector (128 / 0x80)                    */
/* -------------------------------------------------- */

ISR_NOERR 128

.section .note.GNU-stack,"",@progbits