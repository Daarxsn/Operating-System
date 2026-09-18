#include "system_service.h"

int main(void)
{
    xyris_system_snapshot_t snapshot = {0};
    snapshot.version = XYRIS_SYSTEM_SERVICE_VERSION;
    return snapshot.version == 1u ? 0 : 1;
}
