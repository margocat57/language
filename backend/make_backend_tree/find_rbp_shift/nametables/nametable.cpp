#include "nametable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

name_table* NameTableInit(){
    name_table* nametab = (name_table*)calloc(sizeof(name_table), 1);

    variables* nametable_arr = (variables*)calloc(sizeof(variables), MAX_NUMBER_OF_NAMETABLE); 
    if(!nametable_arr){
        fprintf(stderr, "Can't allocate memory for name_table array");
        return nametab;
    }

    nametab->num_of_elements = MAX_NUMBER_OF_NAMETABLE;
    nametab->var_info = nametable_arr;
    nametab->first_free = 0;

    return nametab;
}

size_t NameTableAddName(name_table* nametab, char* num_of_variable){
    assert(nametab);
    assert(num_of_variable);

    nametab->var_info[nametab->first_free].variable_name = strdup(num_of_variable); // уже за стрдюплено
    nametab->first_free++;
    if(nametab->first_free > nametab->num_of_elements){
        NameTableRealloc(nametab, nametab->num_of_elements*2);
    }
    return nametab->first_free - 1;
}

void NameTableRealloc(name_table* nametab, size_t num_of_elem){
    variables* nametable_copy = (variables*)realloc(nametab->var_info, num_of_elem * sizeof(variables));
    if(!nametable_copy){
        fprintf(stderr, "Can't alloc name_table arr\n");
        return;
    }
    nametab->var_info = nametable_copy;
    nametab->num_of_elements = num_of_elem;
    for(size_t nametable = nametab->first_free; nametable < nametab->num_of_elements; nametable++){
        nametab->var_info[nametable].variable_name = NULL;
        nametab->var_info[nametable].is_visible = 0;
    }
}

size_t FindVarInNameTable(name_table* nametab, char* num_of_variable){
    for(size_t name = 0; name < nametab->first_free; name++){
        if(!strcmp(nametab->var_info[name].variable_name, num_of_variable)){
            return name;
        }
    }
    return INT_MAX;
}

void NameTableDestroy(name_table* nametab){
    if(nametab){
        for(size_t i = 0; i < nametab->first_free; i++){
            free(nametab->var_info[i].variable_name);
        }
        if(nametab->var_info){
            free(nametab->var_info);
        }
        free(nametab);
    }
}