#ifndef METKI_H
#define METKI_H
#include <stdlib.h>

const int MAX_NUMBER_OF_METKI = 20;

struct variables{
    char* variable_name;
    double value;
};

struct metki{
    variables* var_info;
    size_t num_of_metki;
    size_t first_free;
};

metki* MetkiInit();

size_t MetkiAddName(metki* mtk, char* num_of_variable);

void MetkiRealloc(metki* mtk, size_t num_of_elem);

size_t FindVarInMtkArr(metki* mtk, char* num_of_variable);

void MetkiAddValues(metki* mtk);

void MetkiDelValues(metki* mtk);

void MetkiDestroy(metki* mtk);

#endif //METKI_H