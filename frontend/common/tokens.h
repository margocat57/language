#ifndef FOREST_H
#define FOREST_H
#include "../../tree/tree.h"
#include "../../tree/mistakes.h"
#include "../nametables/table_of_nametable.h"


struct Tokens_t{
    TreeNode_t** node_arr; // array of top of tree
    table_of_nametable* table;
    size_t num_of_nodes;
    size_t first_free_place;
};

Tokens_t* TokensCtor(size_t num_of_trees);

TreeErr_t TokensAddElem(TreeNode_t *node_add, Tokens_t *tokens);

TreeErr_t TokensRealloc(Tokens_t *tokens, size_t num_of_trees);

TreeErr_t TokensVerify(Tokens_t *tokens);

void TokensDtor(Tokens_t *tokens);

#endif //FOREST_H