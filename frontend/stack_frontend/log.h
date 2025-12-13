//!@file
#ifndef LOG_H
#define LOG_H

//! Opens a file for logging
//! 
//! @param file_name Name of the log file. Can be:
//!                  "stderr" - output to standard error stream (doesn't change current LOG_FILE)
//!                  "stdout" - output to standard output stream
//!                  any other name - file will be opened in append mode ("a+")
//!                  NULL - error message is printed, LOG_FILE remains unchanged
//! 
//! @note If file cannot be opened, error message is printed to stderr
//!       and LOG_FILE remains unchanged
void open_log_file(const char *file_name);

//! Prints formatted string to log file
//! 
//! @param string Format string (like in printf)
//! @param ... Format arguments
//! 
//! @note If NULL is passed, error message is printed to stdout
//! @note For file streams, fflush() is called automatically
void printf_to_log_file(const char* string, ...);

//! Closes the log file
//! 
//! @note Standard streams (stderr/stdout) are not closed
//! @note File streams are closed with fclose()
void close_log_file();

#endif //LOG_H