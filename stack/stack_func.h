//!@file
#ifndef STACK_FUNC_H
#define STACK_FUNC_H
#include <stdint.h>
#include <stdlib.h>
#include "mistakes_code.h"
#include "stack.h"

typedef u_int64_t stack_err_bytes; 

#ifdef _DEBUG
    #define STACK_DUMP(stack) stack_dump(stack);
#else 
    #define STACK_DUMP(stack)
#endif //_DEBUG

#ifdef _DEBUG
    #define DEBUG(...) __VA_ARGS__
#else 
    #define DEBUG(...)
#endif //_DEBUG


//! Stack constructor
//!
//! @param file   Name of the file where the stack is created (for debugging)
//! @param func   Name of the function where the stack is created (for debugging) 
//! @param line   Line number where the stack is created (for debugging)
//!
//! @return Pointer to the created stack or NULL on error
//!
//! @note The function performs:
//!       - Validation of input parameters
//!       - Memory allocation for stack structure and data using calloc function
//!       - Initialization of canaries for stack and data array
//!       - Setting of debug information (file, function, line)
//!       - Calculation of stack and data hash
//!       - Verification of the created stack
//!
//! @warning If parameter check or memory allocation fails,
//!          returns NULL. All allocated resources are freed.
Stack_t* stack_ctor(size_t num_of_elem, const char* file, const char* func, int line);

//! Stack verification function
//!
//! @warning On error detection, logs information and calls STACK_DUMP
//! @note DEBUG checks are performed only in debug mode
//! @note When multiple errors are detected in DEBUG mode, they are accumulated
//!       using bitwise OR (| operator).
//!
//! @details Always performed checks:
//!          - NULL stack pointer
//!          - Stack structure canaries corruption
//!          - Critical data fields validity
//!
//! @details DEBUG checks (error accumulation):
//!          - Stack pointer address bounds
//!          - Pointer alignment correctness
//!          - Stack structure hash consistency
//!          - Data hash consistency
//!          - Data canaries integrity
stack_err_bytes stack_verify(Stack_t* stack);

//! Stack dump function for debugging
//!
//! @param stack Pointer to the stack structure to dump
//!
//! @note Outputs complete information about stack state:
//!       - Stack creation location (file, function, line) in DEBUG mode
//!       - Addresses and main stack parameters
//!       - Canary values and hashes
//!
//! @warning If stack is NULL, outputs error message and terminates operation
//! @note For stacks without null pointers or zero important information, additionally outputs:
//!       - Data canaries dump
//!       - Element-by-element dump of all stack data with addresses using byte representation of each element
void stack_dump(const Stack_t *stack);

//! Stack push function
//!
//! @return func_errors error code (NO_MISTAKE_FUNC on success)
//!
//! @note Performs the following operations:
//!       - Validates stack using stack_verify() and element
//!       - Increases stack size via stack_realloc() if necessary
//!       - Copies element data to stack
//!       - Updates stack top pointer
//!       - Recalculates stack and data array hash using update_stack_data_hash() function
//!       - Validates stack using stack_verify() after element addition
//!
//! @note Element is copied byte-by-byte using memcpy()
stack_err_bytes stack_push(Stack_t* stack, stack_elem_t* elem);

//! Stack pop function
//!
//! @return func_errors error code (NO_MISTAKE_FUNC on success)
//!
//! @note Performs the following operations:
//!       - Validates stack using stack_verify()
//!       - IF NULL pointer to element is passed:
//!         - The last element in the stack is simply removed
//!         - Element memory is filled with zeros
//!       - If valid pointer to element is passed:
//!         - Copies element data to the provided buffer
//!         - Removes element from stack by filling with zeros
//!       - Recalculates stack data hash
//!       - Validates stack after operation using stack_verify()
//!
//! @note Element is copied byte-by-byte using memcpy(), and removed via memset()
stack_err_bytes stack_pop(Stack_t* stack, stack_elem_t* elem);

//! Function to read the top element of the stack without removing it
//!
//! @return func_errors error code (NO_MISTAKE_FUNC on success)
//!
//! @note Performs the following operations:
//!       - Validates stack using stack_verify() before operation
//!       - Checks that elem pointer is not NULL
//!       - Temporarily moves stack pointer backward
//!       - Copies top element to buffer using memcpy()
//!       - Restores stack pointer to original position
//!       - Validates stack using stack_verify() after operation
//!
//! @note Element is copied byte-by-byte using memcpy()
//! @note Does not remove element from stack - only reads it
stack_err_bytes stack_top(Stack_t *stack, stack_elem_t *elem);

//! Stack memory deallocation function
//!
//! @return func_errors error code (NO_MISTAKE_FUNC on success)
//!
//! @note Performs the following operations:
//!       - Validates stack using stack_verify() before deallocation
//!       - Fills stack data memory with zeros using memset()
//!       - Frees memory allocated for data array
//!       - Fills stack structure memory with zeros using memset()
//!       - Frees memory allocated for stack structure
stack_err_bytes stack_free(Stack_t* stack);

#endif //STACK_FUNC_H