#ifndef TREE_FUNC_H
#define TREE_FUNC_H
#include "../include/tree.h"
#include "../include/mistakes.h"
#include <stdio.h>

TreeHead_t* TreeCtor();

TreeNode_t* NodeCtor(VALUE_TYPE type, TreeElem_t data, TreeNode_t* parent, TreeNode_t* left, TreeNode_t* right);

TreeNode_t* NodeCopy(TreeNode_t* node);

TreeErr_t PrintNode(const TreeNode_t* node, FILE* dot_file, int* rank, metki* mtk);

TreeErr_t TreeVerify(const TreeHead_t *head);

TreeErr_t TreeNodeVerify(const TreeNode_t *node);

void TreeDel(TreeHead_t* head);

void TreeDelNodeRecur(TreeNode_t* node);

void NodeDtor(TreeNode_t* node);

#endif //TREE_FUNC_H
