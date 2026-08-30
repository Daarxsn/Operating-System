#include "../include/xyris/process.h"

xyris_syscall_result_t xyris_process_exit(xyris_i32 exit_code)
{
    return xyris_exit(exit_code);
}
