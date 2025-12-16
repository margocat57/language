#ifndef METKI_H
#define METKI_H
#include <stdlib.h>

const int MAX_NUMBER_OF_NAMETABLE = 20;

struct variables{
    char* variable_name;
    bool is_visible;
};

struct name_table{
    variables* var_info;
    size_t num_of_elements;
    int first_free;
};

name_table* NameTableInit();

int NameTableAddName(name_table* mtk, char* num_of_variable);

void NameTableRealloc(name_table* mtk, size_t num_of_elem);

int FindVarInNameTable(name_table* mtk, char* num_of_variable);

void NameTableDestroy(name_table* mtk);

#endif //METKI_H