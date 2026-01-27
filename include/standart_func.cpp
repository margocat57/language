#include "standart_func.h"

func_info_mistake_t find_functions_info_mistake(){
    size_t num_of_op = sizeof(FUNC_INFO) / sizeof(std_func_info);
    size_t len = 0;
    for(size_t idx = 1; idx < num_of_op; idx++){
        if(idx != FUNC_INFO[idx].function){
            fprintf(stderr, "Index %zu and op is %d - not simillar\n", idx, FUNC_INFO[idx].function);
            return INDEX_AND_FUNC_NOT_SIMMILAR;
        }
    }
    return NO_MISTAKE_FUNC;
};