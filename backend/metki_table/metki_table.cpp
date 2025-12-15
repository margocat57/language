#include "metki_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

metki* MetkiInit(){
    metki* mtk = (metki*)calloc(sizeof(metki), 1);

    variables* metki_arr = (variables*)calloc(sizeof(variables), MAX_NUMBER_OF_METKI); 
    if(!metki_arr){
        fprintf(stderr, "Can't allocate memory for metki array");
        return mtk;
    }

    mtk->num_of_metki = MAX_NUMBER_OF_METKI;
    mtk->var_info = metki_arr;
    mtk->first_free = 1;

    return mtk;
}

size_t MetkiAddName(metki* mtk, char* num_of_variable){
    assert(mtk);
    assert(num_of_variable);

    size_t idx = FindVarInMtkArr(mtk, num_of_variable); 
    if(idx != SIZE_MAX){
        return idx;
    }

    mtk->var_info[mtk->first_free].variable_name = strdup(num_of_variable);
    mtk->first_free++;
    if(mtk->first_free > mtk->num_of_metki){
        MetkiRealloc(mtk, mtk->num_of_metki*2);
    }
    return mtk->first_free - 1;
}

void MetkiRealloc(metki* mtk, size_t num_of_elem){
    variables* metki_arr_copy = (variables*)realloc(mtk->var_info, num_of_elem * sizeof(variables));
    if(!metki_arr_copy){
        fprintf(stderr, "Can't alloc metki arr\n");
        return;
    }
    mtk->var_info = metki_arr_copy;
    mtk->num_of_metki = num_of_elem;
    for(size_t metka = mtk->first_free; metka < mtk->num_of_metki; metka++){
        mtk->var_info[metka].variable_name = NULL;
    }
}

size_t FindVarInMtkArr(metki* mtk, char* num_of_variable){
    for(size_t metka = 0; metka < mtk->first_free; metka++){
        if(!strcmp(mtk->var_info[metka].variable_name, num_of_variable)){
            return metka;
        }
    }
    return SIZE_MAX;
}


void MetkiDestroy(metki* mtk){
    if(mtk){
        for(size_t i = 0; i < mtk->first_free; i++){
            free(mtk->var_info[i].variable_name);
        }
        if(mtk->var_info){
            free(mtk->var_info);
        }
        free(mtk);
    }
}