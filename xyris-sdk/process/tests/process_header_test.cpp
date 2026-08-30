#include <xyris/process.h>

int main()
{
    xyris_syscall_result_t (*exit_fn)(xyris_i32) = xyris_process_exit;
    return exit_fn != nullptr ? 0 : 1;
}
