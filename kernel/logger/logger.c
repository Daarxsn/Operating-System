#include "logger.h"

#include "../debug/print.h"

static const char* level_string(log_level_t level)
{
    switch (level)
    {
        case LOG_DEBUG: return "[DEBUG] ";
        case LOG_INFO:  return "[INFO ] ";
        case LOG_WARN:  return "[WARN ] ";
        case LOG_ERROR: return "[ERROR] ";
        case LOG_FATAL: return "[FATAL] ";
        default:        return "[UNKWN] ";
    }
}

void logger_init(void)
{
}

void klog(log_level_t level, const char* message)
{
    debug_print(level_string(level));
debug_print(message);
debug_print("\n");
}

void klog_debug(const char* msg)
{
    klog(LOG_DEBUG, msg);
}

void klog_info(const char* msg)
{
    klog(LOG_INFO, msg);
}

void klog_warn(const char* msg)
{
    klog(LOG_WARN, msg);
}

void klog_error(const char* msg)
{
    klog(LOG_ERROR, msg);
}

void klog_fatal(const char* msg)
{
    klog(LOG_FATAL, msg);
}