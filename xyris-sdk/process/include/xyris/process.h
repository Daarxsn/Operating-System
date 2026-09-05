#ifndef XYRIS_PROCESS_H
#define XYRIS_PROCESS_H
#include <xyris/core.h>
#ifdef __cplusplus
extern "C" {
#endif
xyris_pid_t xyris_process_get_pid(void);
xyris_syscall_result_t xyris_process_exit(xyris_i32 exit_code);
#ifdef __cplusplus
}
#endif
#endif /* XYRIS_PROCESS_H */
