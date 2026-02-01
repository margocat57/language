#include "standart_func.h"

func_info_mistake_t find_functions_info_mistake(){
    for(size_t idx = 1; idx < NUM_OF_STD_FUNC; idx++){
        if(idx != FUNC_INFO[idx].function){
            fprintf(stderr, "Index %zu and op is %d - not simillar\n", idx, FUNC_INFO[idx].function);
            return INDEX_AND_FUNC_NOT_SIMMILAR;
        }
    }
    return NO_MISTAKE_FUNC;
};