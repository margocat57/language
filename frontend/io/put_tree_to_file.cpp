#include <stdio.h>
#include <assert.h>
#include "put_tree_to_file.h"
#include "../common/tree_func.h"
#include "../include/operators_func.h"

#define CALL_FUNC_AND_CHECK_ERR(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return;                                                         \
        } \
    }while(0)

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
// For program as tree to disk

// добавить метки к голове
static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, TreeErr_t* err);

void PutTreeToFile(const char* file_name, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, TreeErr_t* err){
    if(*err) return;
    assert(file_name); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    FILE* fp = fopen(file_name, "w");
    if(!fp){
        fprintf(stderr, "Can't open output file\n");
        *err = CANT_OPEN_OUT_FILE;
        return;
    }

    CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(fp, head->root, head, mtk, err));
    fclose(fp);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, TreeErr_t* err){
    if(*err) return;
    assert(file); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    if(node->type == OPERATOR){
        size_t num_of_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
        if(node->data.op >= num_of_op){
            *err = INCORR_OPERATOR;
            return;
        }
        fprintf(file, "( \"%s\"", OPERATORS_INFO[node->data.op].op_name_dump);
    }
    else if(node->type == CONST){
        fprintf(file, "( \"%lg\"", node->data.const_value);
    }
    else if(node->type == VARIABLE){
        if(node->data.var_code >= mtk->first_free){
            *err = INCORR_IDX_IN_MTK;
            return;
        }
        const char* var = mtk->var_info[node->data.var_code].variable_name;
        if(!var){
            *err = INCORR_IDX_IN_MTK;
            return;
        }
        fprintf(file, "( \"%s\"", var);
    }

    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->left, head, mtk, err));
    } 
    else{
        fprintf(file, " nil");
    }

    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->right, head, mtk, err));
    } 
    else{
        fprintf(file, " nil");
    }

    fprintf(file, " ) ");
    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}