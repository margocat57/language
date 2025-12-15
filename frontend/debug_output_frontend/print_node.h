#ifndef PRINT_NODE_H
#define PRINT_NODE_H
#include "../../tree/tree.h"
#include "../../tree/mistakes.h"
#include "../nametables/nametable.h"
#include <stdio.h>


TreeErr_t PrintNode(const TreeNode_t* node, FILE* dot_file, int* rank, name_table* mtk);

#endif //PRINT_NODE_H