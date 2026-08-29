#include <xyris/core.h>

int main(void)
{
    xyris_syscall_result_t result = XYRIS_OK;
    return xyris_succeeded(result) == XYRIS_TRUE ? 0 : 1;
}
