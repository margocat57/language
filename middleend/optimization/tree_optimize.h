#ifndef CALCUL_TREE_H
#define CALCUL_TREE_H
#include "../../tree/tree.h"
#include "../../tree/mistakes.h"

void CalcTreeExpression(TreeNode_t* node, double* result, TreeErr_t* err);

void TreeOptimize(TreeNode_t **node, TreeErr_t* err);

#endif //CALCUL_TREE_H