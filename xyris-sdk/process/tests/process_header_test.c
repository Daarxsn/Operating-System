#include <xyris/process.h>

int main(void)
{
    xyris_syscall_result_t (*exit_fn)(xyris_i32) = xyris_process_exit;
    return exit_fn != 0 ? 0 : 1;
}
