#ifndef STACK_H
#define STACK_H
#include <stdlib.h>

typedef size_t stack_elem_t; 

const size_t FRONT_CANARY_STACK = 0x4255463132333445;
const size_t TAIL_CANARY_STACK = 0x4E45534255464145; 

const stack_elem_t FRONT_CANARY_DATA = 0xBAAD;
const stack_elem_t TAIL_CANARY_DATA = 0xCAFE;

//! Structure for containing stack
struct Stack_t
{
    size_t front_canary_data;   //!< front data canary
    size_t front_canary;        //!< front stack canary
    size_t top;                 //!< pointer to the top of the stack
    stack_elem_t *data;         //!< array of stack elements
    const char *file;           //!< file where stack was initialized TODO: spell checker
    const char *func;           //!< function where stack was inicialized
    int line;                   //!< line where stack was inicialized
    size_t capacity;            //!< quantity of elements in stack  
    size_t djb2_data;           //!< hash for data array
    size_t djb2_stack;          //!< hash for stack
    size_t tail_canary;         //!< tail stack canary
    size_t tail_canary_data;    //!< tail data canary
};

#endif //STACK_H