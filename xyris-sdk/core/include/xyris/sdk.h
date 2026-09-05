#ifndef XYRIS_SDK_H
#define XYRIS_SDK_H

/* Official Xyris SDK v0.1 public umbrella header. */
#include <xyris/core.h>
#include <xyris/process.h>
#include <xyris/thread.h>
#include <xyris/memory.h>
#include <xyris/filesystem.h>
#include <xyris/ipc.h>
#include <xyris/events.h>
#include <xyris/timers.h>
#include <xyris/devices.h>
#include <xyris/networking.h>
#include <xyris/security.h>

#define XYRIS_SDK_API_VERSION_MAJOR 0u
#define XYRIS_SDK_API_VERSION_MINOR 1u
#define XYRIS_SDK_API_VERSION ((XYRIS_SDK_API_VERSION_MAJOR << 16) | XYRIS_SDK_API_VERSION_MINOR)

#endif
