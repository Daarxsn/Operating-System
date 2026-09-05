#include "../include/xyris/process.h"

xyris_pid_t xyris_process_get_pid(void)
{
    return (xyris_pid_t)xyris_syscall0(XYRIS_SYS_GETPID);
}

xyris_syscall_result_t xyris_process_exit(xyris_i32 exit_code)
{
    return xyris_exit(exit_code);
}
