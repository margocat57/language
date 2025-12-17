//!@file
#ifndef MY_ASSERT_H
#define MY_ASSERT_H
#include "mistakes_code_rbp.h"

#define MY_SAFE_ASSERT(condition)   \
    my_save_assert_func(condition, #condition, __FILE__, __PRETTY_FUNCTION__, __LINE__)

//! Assertion function for condition checking and error logging
//!
//! @return true if assertion failed (condition = false), false otherwise
//!
//! @note When assertion fails, logs detailed error message including:
//!       - File name
//!       - Function name
//!       - Line number
//!       - String representation of the failed condition
bool my_save_assert_func(bool condition, const char* str_condition, const char* file, const char* func, int line);
#endif //MY_ASSERT_H