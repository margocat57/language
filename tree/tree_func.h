#ifndef TREE_FUNC_H
#define TREE_FUNC_H
#include "tree.h"
#include "mistakes.h"
#include <stdio.h>

TreeHead_t* TreeCtor();

TreeNode_t* NodeCtor(VALUE_TYPE type, TreeElem_t data, TreeNode_t* parent, TreeNode_t* left, TreeNode_t* right, char* var_func_name = NULL);

TreeErr_t TreeVerify(const TreeHead_t *head);

TreeErr_t TreeNodeVerify(const TreeNode_t *node);

void TreeDel(TreeHead_t* head);

void TreeDelNodeRecur(TreeNode_t* node);

void NodeDtor(TreeNode_t* node);

#endif //TREE_FUNC_H
