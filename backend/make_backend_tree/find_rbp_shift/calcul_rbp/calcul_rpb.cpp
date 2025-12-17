#include "assert.h"
#define CALCUL_SHIFT
#include "../../../../include/operators_func.h"
#include "../../../../debug_output/graphviz_dump.h"

#define CALL_FUNC_AND_CHECK_ERR(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return;                                                         \
        } \
    }while(0)

#define CALL_FUNC_AND_CHECK_ERR_FREE(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            stack_free(stack);\
            return;                                                         \
        } \
    }while(0)

#define CALL_FUNC_AND_CHECK_ERR_FREE_NAMETAB(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            NameTableDestroy(nametable);\
            return;                                                         \
        } \
    }while(0)

#define CHECK_PARSING_ERR(bad_condition)\
    do{\
    *err = (bad_condition);\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return; \
        } \
    }while(0) \

#define FAIL_IF(bad_condition, err_code, msg)\
    if(bad_condition){ \
        *err = err_code; \
        fprintf(stderr, msg); \
        fprintf(stderr, "\n%s %s %d\n", __FILE__, __func__, __LINE__); \
        return; \
    } \

static void CalculRpbShiftRecursive(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShift(const TreeHead_t* head, TreeErr_t* err){
    if(*err) return;
    assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    Stack_t* stack = stack_ctor(10, __FILE__, __func__, __LINE__);

    size_t num_of_variables = 0;
    name_table* nametable = NULL;
    CALL_FUNC_AND_CHECK_ERR_FREE(CalculRpbShiftRecursive(head->root, stack, nametable, &num_of_variables, err, false));
    stack_free(stack);

    tree_dump_func(head->root, __FILE__, __func__, __LINE__, "RBP shifts counted\n");

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

//-----------------------------------------------------------------------------------------
// Recursive calculations of RBP shift

static void CalculRpbShiftRecursive(TreeNode_t* node, Stack_t* stack, name_table* nametable,  size_t *num_of_variables_init, TreeErr_t* err, bool is_init){
    if(!node) return;
    size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info);
    switch(node->type){
        case INCORR_VAL:                      *err = INCORR_TYPE;                                                                                                                    break;                                                                     
        case FUNCTION: case FUNCTION_MAIN:    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftFuncDecl(node, stack, nametable, num_of_variables_init, err, is_init));                             break; 
        case FUNC_CALL:                       CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftFuncCall(node, stack, nametable, num_of_variables_init, err, is_init));                             break; 
        case CONST:                           CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftOther(node, stack, nametable,  num_of_variables_init, err, is_init));                               break;          
        case VARIABLE:                        CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftVariable(node, stack, nametable, num_of_variables_init, err, is_init));                    break; 
        case OPERATOR:                        if(node->data.op >= num_of_operators) *err = INCORR_OPERATOR;
                                              CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].shift_func(node, stack, nametable, num_of_variables_init, err, is_init));  break;
        default:                              *err = INCORR_TYPE;                                                                                                                    break; 
        }
}

void CalculRpbShiftOther(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable, num_of_variables_init, err, is_init));
}

void CalculRpbShiftFuncCall(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    node->data.var_code = nametable->first_free;

    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));
}

void CalculRpbShiftFuncDecl(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    nametable = NameTableInit();
    for(size_t count = 0; count < *num_of_variables_init; count++){
        CHECK_PARSING_ERR(*err = stack_pop(stack, NULL));
    }
    *num_of_variables_init = 0;
    CALL_FUNC_AND_CHECK_ERR_FREE(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, true));
    CALL_FUNC_AND_CHECK_ERR_FREE_NAMETAB(CalculRpbShiftRecursive(node->right, stack, nametable, num_of_variables_init, err, is_init));
    NameTableDestroy(nametable);
}

void CalculRpbShiftIfElseWhile(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));

    size_t num_of_variables = 0;
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable, &num_of_variables, err, is_init));

    for(size_t idx = 0; idx < num_of_variables; idx++){
        CHECK_PARSING_ERR(*err = stack_pop(stack, NULL));
    }
}

void CalculRpbShiftInit(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, true));
    (*num_of_variables_init)++;
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable,  num_of_variables_init, err, false));
}

//-----------------------------------------------------------------------------------------
// RBP shift variable

static bool FindVarAtStack(TreeNode_t* node, Stack_t* stack, name_table* nametable);

void CalculRpbShiftVariable(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init, TreeErr_t* err, bool is_init){
    if(is_init){
        size_t idx = NameTableAddName(nametable, node->var_func_name);
        CHECK_PARSING_ERR(*err = stack_push(stack, &idx));
        node->data.var_code = idx;
    }
    else{
        bool is_found = FindVarAtStack(node, stack, nametable);

        FAIL_IF(!is_found,
            USE_VAR_BEFORE_INIT,
            "Using variable before init\n");
    }
    free(node->var_func_name);
    node->var_func_name = NULL;

}

static bool FindVarAtStack(TreeNode_t* node, Stack_t* stack, name_table* nametable){
    long long int idx2 = (long long int)stack->top - 1;
    while(idx2 >= 0){
        if(stack->data[idx2] == (long long int)stack->front_canary_data) break;
        if(!strcmp(nametable->var_info[stack->data[idx2]].variable_name, node->var_func_name)){
            node->data.var_code = stack->data[idx2];
            return true;
        }
        idx2--;
    }
    return false;
}