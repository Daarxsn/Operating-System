#include "service_protocol.h"

#include <string.h>

int main(void)
{
    xyris_service_request_t request = {0};
    xyris_service_response_t response = {0};

    request.header.size = sizeof(request);
    request.header.version = XYRIS_SERVICE_PROTOCOL_VERSION;
    request.operation = XYRIS_SERVICE_OP_LOOKUP;
    request.reply_endpoint = (xyris_handle_t)5u;
    strcpy(request.name, "system-service");

    if (!xyris_service_request_valid(&request))
        return 1;

    response.header.size = sizeof(response);
    response.header.version = XYRIS_SERVICE_PROTOCOL_VERSION;
    response.status = XYRIS_SERVICE_REPLY_OK;
    response.version = 1u;
    strcpy(response.name, "system-service");

    if (!xyris_service_response_valid(&response))
        return 2;

    request.operation = XYRIS_SERVICE_OP_LOOKUP;
    request.reply_endpoint = XYRIS_INVALID_HANDLE;
    if (xyris_service_request_valid(&request))
        return 3;

    return 0;
}
