#ifndef PUT_TREE_TO_FILE_H
#define PUT_TREE_TO_FILE_H 

#include "../include/tree.h"
#include "../include/mistakes.h"
#include "../nametables/table_of_nametable.h"

void PutTreeToFile(const char* file_name, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, TreeErr_t* err);

#endif // PUT_TREE_TO_FILE_H