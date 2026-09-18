#ifndef XYRIS_USERSPACE_SERVICE_PROTOCOL_H
#define XYRIS_USERSPACE_SERVICE_PROTOCOL_H

#include <xyris/abi/types.h>

#define XYRIS_SERVICE_PROTOCOL_VERSION 1u
#define XYRIS_SERVICE_PROTOCOL_NAME_MAX 32u

#define XYRIS_SERVICE_OP_REGISTER   1u
#define XYRIS_SERVICE_OP_UNREGISTER 2u
#define XYRIS_SERVICE_OP_LOOKUP     3u

#define XYRIS_SERVICE_REPLY_OK       0
#define XYRIS_SERVICE_REPLY_NOT_FOUND (-1)
#define XYRIS_SERVICE_REPLY_EXISTS    (-2)
#define XYRIS_SERVICE_REPLY_INVALID   (-3)

/*
 * Wire-format contract for the future service-registry IPC endpoint.
 * This header defines data only. It does not assign new ABI syscalls.
 */
typedef struct
{
    xyris_abi_header_t header;
    xyris_u32 operation;
    xyris_u32 version;
    xyris_handle_t endpoint;
    xyris_capability_t capability;
    xyris_handle_t reply_endpoint;
    char name[XYRIS_SERVICE_PROTOCOL_NAME_MAX];
} xyris_service_request_t;

typedef struct
{
    xyris_abi_header_t header;
    xyris_i32 status;
    xyris_u32 version;
    xyris_handle_t endpoint;
    xyris_capability_t capability;
    char name[XYRIS_SERVICE_PROTOCOL_NAME_MAX];
} xyris_service_response_t;

static inline int xyris_service_protocol_name_valid(const char *name)
{
    xyris_u32 index = 0u;

    if (name == 0 || name[0] == '\0')
        return 0;

    while (name[index] != '\0')
    {
        if (index + 1u >= XYRIS_SERVICE_PROTOCOL_NAME_MAX)
            return 0;
        ++index;
    }

    return 1;
}

static inline int xyris_service_request_valid(const xyris_service_request_t *request)
{
    if (request == 0 || request->header.size < sizeof(*request) ||
        request->header.version != XYRIS_SERVICE_PROTOCOL_VERSION ||
        request->operation < XYRIS_SERVICE_OP_REGISTER ||
        request->operation > XYRIS_SERVICE_OP_LOOKUP ||
        !xyris_service_protocol_name_valid(request->name))
        return 0;

    if (request->operation == XYRIS_SERVICE_OP_LOOKUP)
        return request->reply_endpoint != XYRIS_INVALID_HANDLE;

    return request->version != 0u &&
           ((request->endpoint == XYRIS_INVALID_HANDLE) ==
            (request->capability == XYRIS_INVALID_CAP));
}

static inline int xyris_service_response_valid(const xyris_service_response_t *response)
{
    if (response == 0 || response->header.size < sizeof(*response) ||
        response->header.version != XYRIS_SERVICE_PROTOCOL_VERSION ||
        response->status == XYRIS_SERVICE_REPLY_INVALID)
        return 0;

    if (response->status == XYRIS_SERVICE_REPLY_OK &&
        !xyris_service_protocol_name_valid(response->name))
        return 0;

    return 1;
}

#endif /* XYRIS_USERSPACE_SERVICE_PROTOCOL_H */
