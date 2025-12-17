#include "tokens.h"
#include "../../tree/tree_func.h"
#include <stdio.h>
#include <assert.h>

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Forest Ctor

Tokens_t* TokensCtor(size_t num_of_nodes){
    Tokens_t* tokens = (Tokens_t*)calloc(1, sizeof(Tokens_t));
    if(!tokens){
        fprintf(stderr, "Can't alloc memory for head_arr");
        return NULL;
    }

    TreeNode_t** node_arr = (TreeNode_t**)calloc(num_of_nodes, sizeof(TreeNode_t*));
    if(!node_arr){
        fprintf(stderr, "Can't alloc memory for head_arr");
        return NULL;
    }
    tokens->node_arr = node_arr;
    tokens->num_of_nodes = num_of_nodes;
    tokens->first_free_place = 0;
    return tokens;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
// Forest add elem

TreeErr_t TokensAddElem(TreeNode_t *node_add, Tokens_t *tokens){
    TreeErr_t err = NO_MISTAKE;
    DEBUG_TREE(err = TokensVerify(tokens);)
    if(err) return err;

    if(tokens->first_free_place >= tokens->num_of_nodes){
        CHECK_AND_RET_TREEERR(TokensRealloc(tokens, tokens->num_of_nodes * 2));
    }
    tokens->node_arr[tokens->first_free_place] = node_add;
    tokens->first_free_place++;

    DEBUG_TREE(err = TokensVerify(tokens);)
    return err;
}

TreeErr_t TokensRealloc(Tokens_t *tokens, size_t num_of_nodes){
    TreeErr_t err = NO_MISTAKE;
    DEBUG_TREE(err = TokensVerify(tokens);)
    if (err) return err;

    TreeNode_t** node_arr = (TreeNode_t**)realloc(tokens->node_arr, num_of_nodes * sizeof(TreeNode_t*));
    if(!node_arr){
        fprintf(stderr, "Can't alloc realloc for head_arr");
        TokensDtor(tokens);
        return ALLOC_ERR;
    }
    for(size_t idx = tokens->first_free_place; idx < num_of_nodes; idx++){
        node_arr[idx] = NULL;
    }
    tokens->node_arr = node_arr;
    tokens->num_of_nodes = num_of_nodes;

    DEBUG_TREE(err = TokensVerify(tokens);)
    return err;
}

//-------------------------------------------------------------
//--------------------------------------------------------------
// Forest verify

TreeErr_t TokensVerify(Tokens_t* tokens){
    assert(tokens);
    TreeErr_t err = NO_MISTAKE;
    for (size_t idx = 0; idx < tokens->num_of_nodes; idx++){
        err = TreeNodeVerify(tokens->node_arr[idx]);
        if(err) return err;
    }
    return err;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
// Forest dtor

void TokensDtor(Tokens_t* tokens){
    if(!tokens){
        return;
    }

    for(size_t idx = 0; idx < tokens->first_free_place; idx++){
        if(!tokens->node_arr[idx]){
            continue;
        }
        NodeDtor(tokens->node_arr[idx]);
    }
    if(tokens->node_arr){
        free(tokens->node_arr);
    }

    free(tokens);
}