#ifndef XYRIS_THREAD_H
#define XYRIS_THREAD_H

/* Xyris SDK Thread API v0.1 — kernel-backed thread control. */
#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline xyris_bool_t xyris_thread_id_valid(xyris_tid_t tid)
{ return tid != XYRIS_INVALID_TID ? XYRIS_TRUE : XYRIS_FALSE; }

static inline xyris_tid_t xyris_thread_self(void)
{ return (xyris_tid_t)xyris_syscall0(XYRIS_SYS_THREAD_SELF); }

static inline xyris_status_t xyris_thread_yield(void)
{ return xyris_syscall0(XYRIS_SYS_THREAD_YIELD); }

static inline xyris_status_t xyris_thread_sleep(xyris_duration_ns_t duration)
{ return xyris_syscall1(XYRIS_SYS_THREAD_SLEEP, duration / 1000000ULL); }

static inline xyris_status_t xyris_thread_get_info(xyris_tid_t tid, xyris_thread_info_t *info)
{ return xyris_syscall2(XYRIS_SYS_THREAD_INFO, tid, (xyris_syscall_arg_t)(xyris_user_ptr_t)(uintptr_t)info); }

#ifdef __cplusplus
}
#endif
#endif
