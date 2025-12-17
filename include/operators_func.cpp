#include "operators_func.h"

Op_info_mistake_t find_operators_info_mistake(){
    size_t num_of_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
    size_t len = 0;
    for(size_t idx = 1; idx < num_of_op; idx++){
        if(idx != OPERATORS_INFO[idx].op){
            fprintf(stderr, "Index %zu and op is %d - not simillar\n", idx, OPERATORS_INFO[idx].op);
            return INDEX_AND_OP_NOT_SIMMILAR;
        }
        len = strlen(OPERATORS_INFO[idx].op_name_in_code);
        if(len != OPERATORS_INFO[idx].num_of_symb_code){
            fprintf(stderr, "Len of %s is %zu but in table has %zu\n", OPERATORS_INFO[idx].op_name_in_code, len, OPERATORS_INFO[idx].num_of_symb_code);
            return INCORR_LEN;
        }
    }
    return NO_MISTAKE_OP;
};