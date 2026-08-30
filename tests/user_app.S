.section .text
.global _start
.type _start, @function

_start:
    /*
     * Reachable only after a successful Ring-3 iretq.
     *
     * Put a distinctive value in RAX so the debugger/logging
     * path can distinguish actual user execution later.
     */
    mov $0x52594E4733000001, %rax

    /*
     * SYS_EXIT(42)
     */
    mov $4, %rax
    mov $42, %rdi
    int $0x80

1:
    hlt
    jmp 1b

.section .rodata

message:
    .ascii "RING3 USER OK\n"