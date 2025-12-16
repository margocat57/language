#include <stdio.h>
#include "mistakes_code.h"
#include "log.h"

bool my_save_assert_func(bool condition, const char *str_condition, const char *file, const char *func, int line)
{
    if (!condition)
    {
        printf_to_log_file("%s: %s line %d, assertion failed at %s\n", file, func, line, str_condition);
        return true;
    }
    return false;
}