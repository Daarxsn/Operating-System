#ifndef XYRIS_CONTEXT_H
#define XYRIS_CONTEXT_H

#include <stdint.h>

/*
 * ============================================================
 * XyrisOS x86-64 Thread Context
 * ============================================================
 *
 * Layout MUST match kernel/process/switch.S exactly.
 *
 * Offset:
 *   0  = RSP
 *   8  = RBP
 *  16  = RBX
 *  24  = R12
 *  32  = R13
 *  40  = R14
 *  48  = R15
 *  56  = RIP
 *  64  = RFLAGS (saved IF state; switch.S restores IF only)
 *
 * Total size = 72 bytes.
 * ============================================================
 */

typedef struct context
{
    uint64_t rsp;       /* 0  */
    uint64_t rbp;       /* 8  */
    uint64_t rbx;       /* 16 */
    uint64_t r12;       /* 24 */
    uint64_t r13;       /* 32 */
    uint64_t r14;       /* 40 */
    uint64_t r15;       /* 48 */
    uint64_t rip;       /* 56 */
    uint64_t rflags;    /* 64 */

} context_t;


/*
 * Context Manager API.
 */

void context_initialize(void);

void context_save(context_t *context);

void context_restore(context_t *context);

void context_switch(
    context_t *current,
    context_t *next
);

#endif
