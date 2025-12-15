#ifndef METKI_TABLE_H
#define METKI_TABLE_H
#include <stdlib.h>

const int MAX_NUMBER_OF_METKI_TRANSLATE = 20;

struct variables_info{
    char* variable_name;
};

struct metki_for_translate{
    variables_info* var_info;
    size_t num_of_metki;
    size_t first_free;
};

metki_for_translate* MetkiInit();

size_t MetkiAddName(metki_for_translate* mtk, char* num_of_variable);

void MetkiRealloc(metki_for_translate* mtk, size_t num_of_elem);

size_t FindVarInMtkArr(metki_for_translate* mtk, char* num_of_variable);

void MetkiDestroy(metki_for_translate* mtk);

#endif //METKI_TABLE_H