#include <stdio.h>
#include <stdarg.h>
#include "string.h"

static FILE *LOG_FILE = stderr;

void open_log_file(const char *file_name)
{
    if (!strcmp("stderr", file_name))
        return;

    if (!strcmp("stdout", file_name))
    {
        LOG_FILE = stdout;
        return;
    }

    if (!file_name)
    {
        fprintf(stderr, "NULL pointer to file name. Log file will be set to stderr\n");
        return;
    }

    FILE *log_file_ptr = fopen(file_name, "a+");
    if (!log_file_ptr)
    {
        fprintf(stderr, "Can't open log file. Log file will be set to stderr\n");
        return;
    }
    LOG_FILE = log_file_ptr;
}

void printf_to_log_file(const char *string, ...)
{
    if (!string)
    {
        printf("NULL ptr to string, can't be printed");
        return;
    }
    va_list args;
    va_start(args, string);

    vfprintf(LOG_FILE, string, args);

    va_end(args);

    if (LOG_FILE != stderr || LOG_FILE != stdout)
        fflush(LOG_FILE);
}

void close_log_file()
{
    if (LOG_FILE == stderr || LOG_FILE == stdout)
        return;

    fclose(LOG_FILE);
}