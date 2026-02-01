#ifndef STANDART_FUNC_H
#define STANDART_FUNC_H
#include <string.h>
#include "../tree/tree.h"
#include <stdio.h>
#include "../backend/make_backend_tree/find_rbp_shift/calcul_rbp/calcul_rbp.h"
#include "../backend/translate_to_asm/translator.h"

#ifdef CALCUL_SHIFT
    #define SHIFT_RBP(...) __VA_ARGS__
#else 
    #define SHIFT_RBP(...)
#endif //PROCESSOR

#ifdef TRANSLATOR
    #define TRASLATE(...) __VA_ARGS__
#else 
    #define TRASLATE(...)
#endif //PROCESSOR

struct std_func_info{
    // reading from file
    STANDART_FUNCTIONS function;
    const char* func_name_in_code;
    size_t num_of_symb_code;
    bool is_void;
    // syntax parse
    size_t num_of_params;
    // dump
    const char* name_for_graph_dump;
    // shift count
    SHIFT_RBP(void(*shift_func)(TreeNode_t*, Stack_t*, name_table*,  size_t*,  TreeErr_t*, bool);)
    // translator function
    TRASLATE(void(*translate_func)(FILE*, TreeNode_t*, op_counters*, TreeErr_t*);)
};


const std_func_info FUNC_INFO[] = {
    {DRAW_RAM_ELEM,  "Disegnare_un_cazzo", sizeof("Disegnare_un_cazzo") - 1, true, 2, "FUNC_DRAW_PIXEL",  SHIFT_RBP(CalculRpbShiftOther,)  TRASLATE(CreateDrawStdFuncAsm)}
};

const size_t NUM_OF_STD_FUNC = sizeof(FUNC_INFO) / sizeof(std_func_info); 

typedef uint64_t func_info_mistake_t;

enum FUNC_INFO_MISTAKES{
    NO_MISTAKE_FUNC,
    INDEX_AND_FUNC_NOT_SIMMILAR
};

func_info_mistake_t find_functions_info_mistake();


#endif //STANDART_FUNC_H