#include <stdio.h>
#include <assert.h>
#include "put_tree_to_file.h"
#include "../../tree/tree_func.h"
#include "../../include/operators_func.h"
#include "../../include/standart_func.h"

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

static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, TreeErr_t* err);

void PutTreeToFile(const char* file_name, const TreeHead_t* head,TreeErr_t* err){
    if(*err) return;
    assert(file_name); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    FILE* fp = fopen(file_name, "w");
    if(!fp){
        fprintf(stderr, "Can't open output file\n");
        *err = CANT_OPEN_OUT_FILE;
        return;
    }

    CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(fp, head->root, head, err));
    fclose(fp);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

static void PutTreeToFileRecursive(FILE *file, TreeNode_t *node, const TreeHead_t* head, TreeErr_t* err){
    if(*err) return;
    assert(file); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
    static size_t count = 0;

    switch(node->type){
        case INCORR_VAL: *err = INCORR_OPERATOR; break;
        case FUNCTION: case FUNCTION_MAIN:
            count++;
            if(node->type == FUNCTION) fprintf(file, "( \"FUNC %s ", node->var_func_name);
            if(node->type == FUNCTION_MAIN) fprintf(file, "( \"MAIN %s ", node->var_func_name);
            break;
        case OPERATOR:
            if(node->data.op >= NUM_OF_OP){
                *err = INCORR_OPERATOR;
                return;
            }
            fprintf(file, "( \"OP %d ", OPERATORS_INFO[node->data.op].op);
            break;
        case FUNCTION_STANDART_NON_VOID: case FUNCTION_STANDART_VOID:
            if(node->data.stdlib_func >= NUM_OF_STD_FUNC){
                *err = INCORR_OPERATOR;
                return;
            }
            fprintf(file, "( \"STD %d ", FUNC_INFO[node->data.stdlib_func].function);
            break;
        case CONST:
            fprintf(file, "( \"%lg ", node->data.const_value);
            break;
        case VARIABLE:
            fprintf(file, "( \"VAR %s ", node->var_func_name);
            break;
        case FUNC_CALL:
            fprintf(file, "( \"CALL %s ", node->var_func_name);
            break;
        default: *err = INCORR_OPERATOR; break;
    }

    fprintf(file, "line %zu pos %zd\"", node->num_of_str, node->pos_in_str);

    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->left, head, err));
    } 
    else{
        fprintf(file, " nil");
    }

    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(PutTreeToFileRecursive(file, node->right, head, err));
    } 
    else{
        fprintf(file, " nil");
    }

    fprintf(file, " ) ");
    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}