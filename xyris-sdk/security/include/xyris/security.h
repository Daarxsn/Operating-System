#ifndef XYRIS_SECURITY_H
#define XYRIS_SECURITY_H

#include <xyris/core.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XYRIS_SDK_SECURITY_VERSION_MAJOR 0u
#define XYRIS_SDK_SECURITY_VERSION_MINOR 1u
#define XYRIS_SECURITY_RIGHT_ALL (XYRIS_SECURITY_RIGHT_READ | XYRIS_SECURITY_RIGHT_WRITE | XYRIS_SECURITY_RIGHT_EXECUTE | XYRIS_SECURITY_RIGHT_SIGNAL | XYRIS_SECURITY_RIGHT_TRANSFER | XYRIS_SECURITY_RIGHT_ADMIN)

static inline xyris_bool_t xyris_capability_valid(xyris_capability_t capability)
{ return capability != XYRIS_INVALID_CAP ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_security_rights_valid(xyris_u64 rights)
{ return (rights & ~XYRIS_SECURITY_RIGHT_ALL) == 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_security_identity_valid(const xyris_security_identity_t *identity)
{ return identity && identity->header.size >= sizeof(*identity) && identity->header.version != 0u && identity->identity != 0u ? XYRIS_TRUE : XYRIS_FALSE; }
static inline xyris_bool_t xyris_security_policy_valid(const xyris_security_policy_t *policy)
{ return policy && policy->header.size >= sizeof(*policy) && policy->header.version != 0u && xyris_capability_valid(policy->capability) && policy->object != XYRIS_INVALID_OBJECT && xyris_security_rights_valid(policy->allow) && xyris_security_rights_valid(policy->deny) ? XYRIS_TRUE : XYRIS_FALSE; }

static inline xyris_status_t xyris_security_get_identity(xyris_security_identity_t *out)
{ return xyris_syscall1(XYRIS_SYS_SECURITY_IDENTITY, (xyris_user_ptr_t)(uintptr_t)out); }
static inline xyris_status_t xyris_security_check(xyris_capability_t capability, xyris_object_id_t object, xyris_u64 rights)
{ return xyris_syscall3(XYRIS_SYS_SECURITY_CHECK, capability, object, rights); }

#ifdef __cplusplus
}
#endif
#endif
