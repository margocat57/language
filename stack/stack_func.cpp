//!@file
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stack_func.h"
#include "hash.h"
#include "mistakes_code.h"
#include "my_assert.h"
#include "log.h"
#include "stack.h"

const char* FORM_SPEC_STK_EL = "%lg";
const int POISON = 6666;
const double SIZE_INCREASE = 2;

//! Memory for canaries in data array
const size_t CANARY_ELEMS = 2;

//! Stack memory expansion function
//!
//! @return func_errors error code (NO_MISTAKE_FUNC on success)
//!
//! Performs the following operations:
//!    - Validates stack before operation using stack_verify()
//!    - Saves current stack pointer position
//!    - Expands data array memory using realloc()
//!    - Initializes new memory with zeros
//!    - Doubles stack capacity and updates pointer position
//!    - Restores data tail canary
//!    - Recalculates stack hashes
//!    - Validates stack after operation using stack_verify()
//!
//! Returns ALLOC_ERROR if realloc() fails
static stack_err_bytes stack_realloc(Stack_t *stack);

//! Stack hash update function 
//!
//! Calculates two separate hashes:
//!     - djb2_stack: Hash of stack structure (from front_canary to tail_canary)
//!     - djb2_data: Hash of data array (including reserved areas)
//! @note Hash calculation includes canaries 
static void update_stack_data_hash(Stack_t *stack);

//! Fill all stack elements at poison value
static stack_err_bytes fill_poison(Stack_t* stack, size_t start, size_t end);

Stack_t *stack_ctor(size_t num_of_elem, const char *file, const char *func, int line)
{
    if (MY_SAFE_ASSERT(num_of_elem < (size_t)(0.8 * (double)SIZE_MAX)))
        return NULL;

    Stack_t *stack = (Stack_t *)calloc(1, sizeof(Stack_t));
    if (MY_SAFE_ASSERT(stack != NULL))
        return NULL;

    size_t real_size = num_of_elem DEBUG (+ CANARY_ELEMS);
    stack->data = (stack_elem_t *)calloc(real_size, sizeof(stack_elem_t));
    if (MY_SAFE_ASSERT(stack->data != NULL))
    {
        free(stack);
        return NULL;
    }

    stack->front_canary = FRONT_CANARY_STACK;
    stack->tail_canary = TAIL_CANARY_STACK;
    stack->front_canary_data = FRONT_CANARY_DATA;
    stack->tail_canary_data = TAIL_CANARY_DATA;
    stack->top = 0 DEBUG(+ CANARY_ELEMS/2); 
    stack->capacity = num_of_elem;
    DEBUG(
    stack->file = file;
    stack->func = func;
    stack->line = line;
    )

    DEBUG(
    *stack->data = FRONT_CANARY_DATA;
    (stack->data)[stack->capacity - 1 + CANARY_ELEMS] = TAIL_CANARY_DATA;
    )

    fill_poison(stack, 0 DEBUG(+ CANARY_ELEMS / 2) , stack->capacity - 1  DEBUG(+ CANARY_ELEMS / 2));

    DEBUG(update_stack_data_hash(stack);)

    if (MY_SAFE_ASSERT(stack_verify(stack) == NO_MISTAKE_STACK))
        return NULL;
    

    return stack;
}

static stack_err_bytes fill_poison(Stack_t* stack, size_t start, size_t end){
    if(MY_SAFE_ASSERT(stack->top <= start && start < stack->capacity + CANARY_ELEMS)) 
        return INCORR_DIGIT_PARAMS;
    if(MY_SAFE_ASSERT(stack->top <= end && end < stack->capacity + CANARY_ELEMS)) 
        return INCORR_DIGIT_PARAMS;

    for (size_t index = start; index <= end; index++)
    {
        stack->data[index] = POISON;
    }
    return NO_MISTAKE_STACK;
}

//  cкопировать чтобы не менять стек
stack_err_bytes stack_verify(Stack_t *stack)
{
    stack_err_bytes error = NO_MISTAKE_STACK;

    if (!stack)
    {
        printf_to_log_file("NULL stack pointer\n");
        return NULL_STACK_PTR;
    }

    if (stack->front_canary != FRONT_CANARY_STACK || 
        stack->tail_canary  != TAIL_CANARY_STACK)
    {
        printf_to_log_file("CANARIES of stack CORRUPTED\n");
        return CANARY_ST_NOT_IN_PLACES;
    }

    if (!stack->data || stack->capacity == 0)
    {
        printf_to_log_file("Stack important data is zero or null-pointed\n");
        STACK_DUMP(stack);
        return NULL_STACK_PTR;
    }

    if (stack->top < 0 DEBUG( + CANARY_ELEMS / 2 - 1)){
        printf_to_log_file("Stack pointer adress smaller than data adress\n");
        STACK_DUMP(stack);
        return PTR_SMALLER_THAN_DATA;
    }

    if (stack->top > stack->capacity - 1 DEBUG( + CANARY_ELEMS)) {
        printf_to_log_file("Stack pointer adress bigger than data adress\n");
        STACK_DUMP(stack);
        return PTR_BIGGER_THAN_DATA;
    }

    DEBUG(
    size_t stack_old = stack->djb2_stack;
    size_t data_old = stack->djb2_data;
    update_stack_data_hash(stack);

    if (stack_old != stack->djb2_stack) {
        printf_to_log_file("Stack hash is not correct\n");
        STACK_DUMP(stack);
        error = error | STACK_HASH_NOT_CORRECT;
    }

    stack->djb2_data = create_djb2_hash((char*)stack->data, stack->capacity + CANARY_ELEMS);
    if (data_old != stack->djb2_data) {
        printf_to_log_file("Stack data hash is not correct\n");
        STACK_DUMP(stack);
        error = error | DATA_HASH_NOT_CORRECT;
    }
    )

    DEBUG(
    stack_elem_t tail_canary = (stack->data)[CANARY_ELEMS + stack->capacity - 1];
    if (tail_canary != TAIL_CANARY_DATA || *(stack->data) != FRONT_CANARY_DATA) {
        printf("DATA CANARIES DEAD\n");
        STACK_DUMP(stack);
        error = error | CANARY_DT_NOT_IN_PLACES;
    }
    )

    return error;
}

void stack_dump(const Stack_t *stack)
{
    if (!stack)
    {
        printf_to_log_file("Stack can't be printed due to NULL ptr\n");
        return;
    }

    DEBUG(printf_to_log_file("Stack was created at %s %s %d\n", stack->file, stack->func, stack->line);)

    printf_to_log_file("Stack adress %p\n", stack);
    printf_to_log_file("Front stack canary: %X\n", stack->front_canary);
    printf_to_log_file("Data adress: %p\n", stack->data);
    printf_to_log_file("Pointer points at: %lu\n", stack->top);
    printf_to_log_file("Stack capacity: %lu\n", stack->capacity);

    DEBUG(
    printf_to_log_file("DJB2 hash for data array: %lu\n", stack->djb2_data);
    printf_to_log_file("DJB2 hash for whole stack: %lu\n", stack->djb2_stack);
    )

    printf_to_log_file("Tail stack canary: %X\n", stack->tail_canary);


    if (!stack->data || stack->capacity == 0){
        printf_to_log_file("\n");
        return;
    }

    for (size_t idx = 0; idx <= stack->capacity DEBUG(+ CANARY_ELEMS) - 1; idx++)
    {
    if (stack->data[idx] == POISON)
    {
        printf_to_log_file("[%lu] = ", idx);
        printf_to_log_file(FORM_SPEC_STK_EL, stack->data[idx]);
        printf_to_log_file( " (POISON)\n");
    }
    DEBUG(
    else if (stack->data[idx] == TAIL_CANARY_DATA)
    {
        printf_to_log_file("[%lu] = ", idx);
        printf_to_log_file( FORM_SPEC_STK_EL, stack->data[idx]);
        printf_to_log_file(" (TAIL DATA CANARY)\n");
    }
    else if (stack->data[idx] == FRONT_CANARY_DATA)
    {
        printf_to_log_file("[%lu] = ", idx);
        printf_to_log_file(FORM_SPEC_STK_EL, stack->data[idx]);
        printf_to_log_file(" (FRONT DATA CANARY)\n");
    })
    else
    {
        printf_to_log_file("^[%lu] = ", idx);
        printf_to_log_file( FORM_SPEC_STK_EL, stack->data[idx]);
        printf_to_log_file("\n", idx);
    }
    }
    printf_to_log_file("\n");
}

static void update_stack_data_hash(Stack_t *stack)
{
    stack->djb2_stack = 0;
    stack->djb2_data = 0;
    size_t size = (size_t)(&stack->tail_canary - &stack->front_canary) + sizeof(stack->tail_canary);
    stack->djb2_stack = create_djb2_hash((char *)stack, size);
    stack->djb2_data = create_djb2_hash((char *)stack->data, stack->capacity + CANARY_ELEMS);
}

// неалгоритм дефайн
stack_err_bytes stack_push(Stack_t *stack, stack_elem_t *elem)
{
    stack_err_bytes stack_verify_error = stack_verify(stack);
    MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK);
    if (MY_SAFE_ASSERT(elem != NULL))
        stack_verify_error = stack_verify_error | FUNC_PARAM_IS_NULL;
    if (stack_verify_error)
        return stack_verify_error;

    if (stack->top + 1 >= stack->capacity DEBUG(+ CANARY_ELEMS/2))
    {  
        DEBUG(update_stack_data_hash(stack);)
        stack_realloc(stack);
    }
    stack->data[stack->top] = *elem;
    stack->top++;

    DEBUG(update_stack_data_hash(stack);)

    stack_verify_error = stack_verify(stack);
    MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK);
    return stack_verify_error;
}

stack_err_bytes stack_pop(Stack_t *stack, stack_elem_t *elem)
{   
    stack_err_bytes stack_verify_error = stack_verify(stack);
    if (MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK))
        return stack_verify_error;
    if (!elem)
    {
        if (stack->top <= 0 DEBUG(+ CANARY_ELEMS / 2))
            return INCORR_POP_IDX;
        stack->top--;
        stack->data[stack -> top] = POISON;
        DEBUG(update_stack_data_hash(stack);)
        return NO_MISTAKE_STACK;
    }

    if (stack->top <= 0 DEBUG(+ CANARY_ELEMS / 2))
        return INCORR_POP_IDX;
    stack->top--;
    *elem = stack->data[stack -> top];
    stack->data[stack -> top] = POISON;


    DEBUG(update_stack_data_hash(stack);)

    stack_verify_error = stack_verify(stack);
    MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK);
    return stack_verify_error;
}

stack_err_bytes stack_top(Stack_t *stack, stack_elem_t *elem)
{
    stack_err_bytes stack_verify_error = stack_verify(stack);
    MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK);
    if (MY_SAFE_ASSERT(elem != NULL))
        stack_verify_error = stack_verify_error | FUNC_PARAM_IS_NULL;
    if (stack_verify_error)
        return stack_verify_error;

    if (stack->top <= 0 DEBUG(+ CANARY_ELEMS / 2))
        return INCORR_POP_IDX; 
    *elem = stack->data[stack -> top - 1];

    return stack_verify_error;
}

// частный случай стека - стек ноль, указатель на дату 0, дешевое создание нулвеовог стека
static stack_err_bytes stack_realloc(Stack_t *stack)
{   
    stack_err_bytes stack_verify_error = stack_verify(stack);
    if (MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK))
        return stack_verify_error;

    size_t old_capacity = stack->capacity;
    // + 1
    stack->capacity = (size_t)((double)stack->capacity * SIZE_INCREASE);

    stack_elem_t* new_data = (stack_elem_t*)realloc(stack->data, (stack->capacity DEBUG(+ CANARY_ELEMS))*sizeof(stack_elem_t));
    if (MY_SAFE_ASSERT(new_data != NULL))
        return ALLOC_ERROR;

    stack->data = new_data;
    fill_poison(stack, old_capacity DEBUG (+ CANARY_ELEMS) - 1, stack->capacity - 1 DEBUG (+ CANARY_ELEMS / 2));

    DEBUG(
    stack->data[CANARY_ELEMS + stack->capacity - 1] = TAIL_CANARY_DATA;
    )

    DEBUG(update_stack_data_hash(stack);)

    stack_verify_error = stack_verify(stack);
    MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK);
    return stack_verify_error;
}


stack_err_bytes stack_free(Stack_t *stack)
{   
    stack_err_bytes stack_verify_error = stack_verify(stack);
    if (MY_SAFE_ASSERT(stack_verify_error == NO_MISTAKE_STACK))
        return stack_verify_error;

    free(stack->data);
    stack->data = NULL;
    free(stack);

    return stack_verify_error;
}