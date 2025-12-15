//!@file
#ifndef MISTAKES_CODE_H
#define MISTAKES_CODE_H

//! Mistakes at verification and function's work
enum stack_err_t {
    NO_MISTAKE_STACK        = 0,
    NULL_STACK_PTR          = 1 << 0,
    NULL_STACK_ELEM         = 1 << 1,
    CANARY_ST_NOT_IN_PLACES = 1 << 2,
    PTR_SMALLER_THAN_DATA   = 1 << 3,
    PTR_BIGGER_THAN_DATA    = 1 << 4,
    ALIGN_NOT_CORRECT       = 1 << 5,
    STACK_HASH_NOT_CORRECT  = 1 << 6,
    DATA_HASH_NOT_CORRECT   = 1 << 7,
    CANARY_DT_NOT_IN_PLACES = 1 << 8,
    //! Mistakes at function's work
    FUNC_PARAM_IS_NULL      = 1 << 9,  //!< Parametr given to function is NULL
    ALLOC_ERROR             = 1 << 10, //!< Stack verification failed
    INCORR_DIGIT_PARAMS     = 1 << 11,  //!< Allocation error
    INCORR_POP_IDX          = 1 << 12,
    //!< Mistakes at processor work
    INCORR_BYTECODE_PTR          =  0x8000000000002000ULL, // (1 << 63) | (1 << 13),
    INCORR_PROCESSOR_PTR         =  0x8000000000004000ULL, // (1 << 63) | (1 << 14),
    INCORR_BYTECODE_ELEM_PTR     =  0x8000000000008000ULL, // (1 << 63) | (1 << 15),
    INCORR_COMAND                =  0x8000000000010000ULL, // (1 << 63) | (1 << 16),
    REGISTR_OVERFLOW             =  0x8000000000020000ULL, // (1 << 63) | (1 << 17),  
    RAM_OVERFLOW                 =  0x8000000000040000ULL, // (1 << 63) | (1 << 18), 
    ZERO_DIV                     =  0x8000000000080000ULL, // (1 << 63) | (1 << 19), 
};



#endif //MISTAKES_CODE_H