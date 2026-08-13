#ifndef XYRIS_COMPILER_H
#define XYRIS_COMPILER_H

/* ============================================================
   Compiler Detection
   ============================================================ */

#if defined(__clang__)
    #define XYRIS_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define XYRIS_COMPILER_GCC 1
#else
    #error "Unsupported compiler."
#endif

/* ============================================================
   Function Attributes
   ============================================================ */

#define PACKED              __attribute__((packed))
#define ALIGNED(x)          __attribute__((aligned(x)))
#define SECTION(x)          __attribute__((section(x)))

#define USED                __attribute__((used))
#define UNUSED              __attribute__((unused))

#define WEAK                __attribute__((weak))

#define NORETURN            __attribute__((noreturn))
#define NOINLINE            __attribute__((noinline))
#define ALWAYS_INLINE       inline __attribute__((always_inline))

#define PURE                __attribute__((pure))
#define CONST_FN            __attribute__((const))

/* ============================================================
   Branch Prediction
   ============================================================ */

#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0)

/* ============================================================
   Compile-Time Helpers
   ============================================================ */

#define STATIC_ASSERT(expr, msg) \
    _Static_assert((expr), msg)

#define OFFSET_OF(type, member) \
    __builtin_offsetof(type, member)

/* ============================================================
   Compiler Barriers
   ============================================================ */

#define COMPILER_BARRIER() \
    __asm__ __volatile__("" ::: "memory")

/* ============================================================
   CPU Relax Hint
   ============================================================ */

static ALWAYS_INLINE void cpu_relax(void)
{
    __asm__ __volatile__("pause");
}

/* ============================================================
   Unreachable Code
   ============================================================ */

#define UNREACHABLE() \
    __builtin_unreachable()

/* ============================================================
   Debug Breakpoint
   ============================================================ */

static ALWAYS_INLINE void debug_break(void)
{
    __asm__ __volatile__("int3");
}

/* ============================================================
   Memory Barriers
   ============================================================ */

static ALWAYS_INLINE void memory_barrier(void)
{
    __sync_synchronize();
}

/* ============================================================
   Interrupt Control (x86_64)
   ============================================================ */

static ALWAYS_INLINE void enable_interrupts(void)
{
    __asm__ __volatile__("sti");
}

static ALWAYS_INLINE void disable_interrupts(void)
{
    __asm__ __volatile__("cli");
}

static ALWAYS_INLINE void halt_cpu(void)
{
    __asm__ __volatile__("hlt");
}

/* ============================================================
   Read CPU Flags
   ============================================================ */

static ALWAYS_INLINE unsigned long read_rflags(void)
{
    unsigned long flags;

    __asm__ __volatile__(
        "pushfq\n\t"
        "pop %0"
        : "=r"(flags));

    return flags;
}

#endif /* XYRIS_COMPILER_H */