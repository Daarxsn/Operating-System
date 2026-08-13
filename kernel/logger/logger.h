#ifndef XYRIS_LOGGER_H
#define XYRIS_LOGGER_H

typedef enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

void logger_init(void);

void klog(log_level_t level, const char* message);

void klog_debug(const char* message);
void klog_info(const char* message);
void klog_warn(const char* message);
void klog_error(const char* message);
void klog_fatal(const char* message);

#endif