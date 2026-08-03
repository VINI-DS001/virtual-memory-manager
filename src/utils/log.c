#include "vmm/utils/log.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>

static log_level_t current_log_level = LOG_LEVEL_DEBUG;

// ANSI Color Codes
#define COLOR_RESET "\033[0m"
#define COLOR_DEBUG "\033[36m" // Cyan
#define COLOR_INFO "\033[32m"  // Green
#define COLOR_WARN "\033[33m"  // Yellow
#define COLOR_ERROR "\033[31m" // Red

static const char *level_strings[] = {
    "DEBUG", "INFO ", "WARN ", "ERROR"};

static const char *level_colors[] = {
    COLOR_DEBUG, COLOR_INFO, COLOR_WARN, COLOR_ERROR};

void log_set_level(log_level_t level)
{
    current_log_level = level;
}

void log_message(log_level_t level, const char *file, int line, const char *fmt, ...)
{
    if (level < current_log_level)
    {
        return;
    }

    time_t rawtime;
    struct tm *timeinfo;
    char time_buf[16];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timeinfo);

    // Get basename of file
    const char *filename = strrchr(file, '/');
    if (!filename)
        filename = strrchr(file, '\\');
    if (filename)
        filename++;
    else
        filename = file;

    fprintf(stderr, "%s[%s] [%s] (%s:%d): ",
            level_colors[level],
            time_buf,
            level_strings[level],
            filename,
            line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", COLOR_RESET);
}