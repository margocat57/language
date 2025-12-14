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
static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, table_of_nametable* table, TreeErr_t* err);

void PutTreeToFile(const char* file_name, const TreeHead_t* head, table_of_nametable* table,  TreeErr_t* err){
    if(*err) return;
    assert(file_name); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    FILE* fp = fopen(file_name, "w");
    if(!fp){
        fprintf(stderr, "Can't open output file\n");
        *err = CANT_OPEN_OUT_FILE;
        return;
    }

    CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(fp, head->root, head, NULL, table, err));
    fclose(fp);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, name_table* mtk, table_of_nametable* table, TreeErr_t* err){
    if(*err) return;
    assert(file); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
    static size_t count = 0;

    if(node->type == FUNCTION){
        mtk = table->nametables[count];
        count++;
        fprintf(file, "( \"FUNC %s \"", node->var_func_name);
    }
    else if(node->type == OPERATOR){
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
        if(!mtk){
            *err = NULL_MTK_PTR;
            return;
        }
        if(node->data.var_code >= mtk->first_free){
            *err = INCORR_IDX_IN_MTK;
            return;
        }
        fprintf(file, "( \"VAR %zu\"", node->data.var_code);
    }
    else if(node->type == FUNC_CALL){
        if(!mtk){
            *err = NULL_MTK_PTR;
            return;
        }
        fprintf(file, "( \"CALL[%zu] %s \"", mtk->first_free, node->var_func_name);
    }

    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->left, head, mtk, table, err));
    } 
    else{
        fprintf(file, " nil");
    }

    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->right, head, mtk, table, err));
    } 
    else{
        fprintf(file, " nil");
    }

    fprintf(file, " ) ");
    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}