#ifndef TRASLATOR_H
#define TRASLATOR_H
#include "../../tree/tree_func.h"
#include "../metki_table/metki_table.h"

void CreateAsmCode(const char* file_name, const TreeHead_t* head, TreeErr_t* err);

#endif //TRASLATOR_H