#include "metki_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

metki_for_translate* MetkiInit(){
    metki_for_translate* mtk = (metki_for_translate*)calloc(sizeof(metki_for_translate), 1);

    variables_info* metki_arr = (variables_info*)calloc(sizeof(variables_info), MAX_NUMBER_OF_METKI_TRANSLATE); 
    if(!metki_arr){
        fprintf(stderr, "Can't allocate memory for metki_for_translate array");
        return mtk;
    }

    mtk->num_of_metki = MAX_NUMBER_OF_METKI_TRANSLATE;
    mtk->var_info = metki_arr;
    mtk->first_free = 1;

    return mtk;
}

size_t MetkiAddName(metki_for_translate* mtk, char* num_of_variable){
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

void MetkiRealloc(metki_for_translate* mtk, size_t num_of_elem){
    variables_info* metki_arr_copy = (variables_info*)realloc(mtk->var_info, num_of_elem * sizeof(variables_info));
    if(!metki_arr_copy){
        fprintf(stderr, "Can't alloc metki_for_translate arr\n");
        return;
    }
    mtk->var_info = metki_arr_copy;
    mtk->num_of_metki = num_of_elem;
    for(size_t metka = mtk->first_free; metka < mtk->num_of_metki; metka++){
        mtk->var_info[metka].variable_name = NULL;
    }
}

size_t FindVarInMtkArr(metki_for_translate* mtk, char* num_of_variable){
    for(size_t metka = 0; metka < mtk->first_free; metka++){
        if(!strcmp(mtk->var_info[metka].variable_name, num_of_variable)){
            return metka;
        }
    }
    return SIZE_MAX;
}


void MetkiDestroy(metki_for_translate* mtk){
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