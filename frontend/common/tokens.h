#ifndef FOREST_H
#define FOREST_H
#include "../include/tree.h"
#include "../include/mistakes.h"


struct Tokens_t{
    TreeNode_t** node_arr; // array of top of tree
    name_table* mtk;
    size_t num_of_nodes;
    size_t first_free_place;
};

Tokens_t* TokensCtor(size_t num_of_trees);

TreeErr_t TokensAddElem(TreeNode_t *node_add, Tokens_t *tokens);

TreeErr_t TokensRealloc(Tokens_t *tokens, size_t num_of_trees);

TreeErr_t TokensVerify(Tokens_t *tokens);

void TokensDtor(Tokens_t *tokens);

#endif //FOREST_H