#ifndef PUT_TREE_TO_FILE_H
#define PUT_TREE_TO_FILE_H 

#include "../../tree/tree.h"
#include "../../tree/mistakes.h"

void PutTreeToFile(const char* file_name, const TreeHead_t* head, TreeErr_t* err);

#endif // PUT_TREE_TO_FILE_H