//!@file
#ifndef MISTAKES_CODE_H
#define MISTAKES_CODE_H

//! Mistakes at verification and function's work
enum stack_err_t {
    NO_MISTAKE_STACK              = 0,
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
    INCORR_POP_IDX          = 1 << 12
};



#endif //MISTAKES_CODE_H