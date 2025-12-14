#include <stdio.h>
#include "table_of_nametable.h"

size_t MAX_NUMBER_OF_NAMETABLES = 10;

table_of_nametable* TableOfNameTableInit(){
    table_of_nametable* nametab = (table_of_nametable*)calloc(sizeof(table_of_nametable), 1);

    name_table** nametables = (name_table**)calloc(sizeof(name_table*), MAX_NUMBER_OF_NAMETABLES); 
    if(!nametables){
        fprintf(stderr, "Can't allocate memory for name_table array");
        return nametab;
    }

    nametab->num_of_nametables = MAX_NUMBER_OF_NAMETABLES;
    nametab->nametables = nametables;
    nametab->first_free = 0;

    return nametab;
}

size_t TableAddName(table_of_nametable* table, name_table* nametable){

    table->nametables[table->first_free] = nametable; // уже за стрдюплено
    (table->first_free)++;
    if(table->first_free > table->num_of_nametables){
        TableRealloc(table, table->num_of_nametables*2);
    }
    return table->first_free - 1;
}

void TableRealloc(table_of_nametable* table, size_t num_of_elem){
    name_table** nametables_copy = (name_table**)realloc(table->nametables, num_of_elem * sizeof(name_table*));
    if(!nametables_copy){
        fprintf(stderr, "Can't alloc name_table arr\n");
        return;
    }
    table->nametables = nametables_copy;
    table->num_of_nametables = num_of_elem;
    for(size_t nametable = table->first_free; nametable < table->num_of_nametables; nametable++){
        table->nametables[nametable] = NULL;
    }
}

void TableDestroy(table_of_nametable* table){
    if(table){
        for(size_t i = 0; i < table->first_free; i++){
            NameTableDestroy(table->nametables[i]);
        }
        if(table->nametables){
            free(table->nametables);
        }
        free(table);
    }
}