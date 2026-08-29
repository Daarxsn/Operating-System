#include <xyris/core.h>

int main()
{
    xyris_syscall_result_t result = XYRIS_OK;
    return xyris_succeeded(result) == XYRIS_TRUE ? 0 : 1;
}
