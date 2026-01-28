#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tree_func.h"
#include "mistakes.h"
#include "tree.h"


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Tree and node constructors

TreeHead_t* TreeCtor(){
    TreeHead_t* head = (TreeHead_t*)calloc(1, sizeof(TreeHead_t));
    head->root = NULL;
    
    return head;
}

TreeNode_t* NodeCtor(VALUE_TYPE type, TreeElem_t data, TreeNode_t* parent, TreeNode_t* left, TreeNode_t* right, char* var_func_name){
    TreeNode_t* node = (TreeNode_t*)calloc(1, sizeof(TreeNode_t));
    if(!node){
        fprintf(stderr, "Can't alloc data for node");
        return NULL;
    }
    node->type = type;
    node->data = data; 
    node->var_func_name = var_func_name;
    node->left = left;
    node->right = right;
    node->parent = parent;
    node->signature = TREE_SIGNATURE;

    return node;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Verifying (recursive algorithm for verifying subtree)

TreeErr_t TreeVerify(const TreeHead_t* head){
    return TreeNodeVerify(head->root);
}

TreeErr_t TreeNodeVerify(const TreeNode_t *node){
    if(!node){
        fprintf(stderr, "NULL node ptr\n");
        return NULL_NODE_PTR;
    }
    if(node->signature != TREE_SIGNATURE){
        fprintf(stderr, "Incorr signature\n");
        return INCORRECT_SIGN;
    }

    if(node->left && node != node->left->parent){
        fprintf(stderr, "Incorr connection between parent(%p) and LEFT child(%p) nodes\n", node, node->left);
        return INCORR_LEFT_CONNECT;
    }
    if(node->right && node != node->right->parent){
        fprintf(stderr, "Incorr connection between parent(%p) and RIGHT child(%p) nodes\n", node, node->right);
        return INCORR_RIGHT_CONNECT;
    }
    if(node->left && node->left == node->right){
        fprintf(stderr, "LOOPED NODE - same connection for left and right");
        return LOOPED_NODE;
    }
    if(node->type == OPERATOR && (node->data.op == OP_ADD || node->data.op == OP_SUB || node->data.op == OP_DIV || node->data.op == OP_MUL || node->data.op == OP_DEG) && (!node->left || !node->right)){
        fprintf(stderr, "No element for binary operator");
        return NO_ELEM_FOR_BINARY_OP;
    }

    if(node->left){
        TreeErr_t err = NO_MISTAKE;
        DEBUG_TREE(err = TreeNodeVerify(node->left);)
        if (err) return err;
    }
    if(node->right){
        TreeErr_t err = NO_MISTAKE;
        DEBUG_TREE(err = TreeNodeVerify(node->right);)
        if(err) return err;
    }

    return NO_MISTAKE;
}


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// Tree and node destructors


void TreeDel(TreeHead_t* head){
    assert(head);

    TreeDelNodeRecur(head->root);

    memset(head, 0, sizeof(TreeHead_t));
    free(head);

}

void TreeDelNodeRecur(TreeNode_t* node){
    if(!node){
        return;
    }

    if(node->left){
        TreeDelNodeRecur(node->left);
    }
    if(node->right){
        TreeDelNodeRecur(node->right);
    }

    NodeDtor(node);
}

void NodeDtor(TreeNode_t* node){
    if(node){
        if(node->var_func_name){
            free(node->var_func_name);
            node->var_func_name = NULL;
        }
        free(node);
    }
}

void NodeDtorPtr(TreeNode_t** node){
    if(node && *node){
        if((*node)->var_func_name){
            free((*node)->var_func_name);
            (*node)->var_func_name = NULL;
        }
        free((*node));
    }
    *node = NULL;
}