#include "assert.h"
#define CALCUL_SHIFT
#include "../../../../include/operators_func.h"
#include "../../../../include/standart_func.h"
#include "../../../../debug_output/graphviz_dump.h"

#define RED                        "\033[1;31m"
#define RESET                      "\033[0m"

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

    // tree_dump_func(head->root, __FILE__, __func__, __LINE__, "RBP shifts counted\n");

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

//-----------------------------------------------------------------------------------------
// Recursive calculations of RBP shift

static void CalculRpbShiftRecursive(TreeNode_t* node, Stack_t* stack, name_table* nametable,  size_t *num_of_variables_init, TreeErr_t* err, bool is_init){
    if(!node) return;

    static size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info);
    static size_t num_of_func = sizeof(FUNC_INFO) / sizeof(std_func_info);
    switch(node->type){
        case INCORR_VAL:                      *err = INCORR_TYPE;                                                                                                                  break;                                                                     
        case FUNCTION: case FUNCTION_MAIN:    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftFuncDecl(node, stack, nametable, num_of_variables_init, err, is_init));                        break; 
        case FUNC_CALL:                       CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftFuncCall(node, stack, nametable, num_of_variables_init, err, is_init));                        break; 
        case CONST:                           CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftOther(node, stack, nametable,  num_of_variables_init, err, is_init));                          break;          
        case VARIABLE:                        CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftVariable(node, stack, nametable, num_of_variables_init, err, is_init));                        break; 
        case FUNCTION_STANDART_VOID:          
        case FUNCTION_STANDART_NON_VOID:      if(node->data.stdlib_func >= num_of_func) *err = INCORR_OPERATOR;
                                              CALL_FUNC_AND_CHECK_ERR(FUNC_INFO[node->data.stdlib_func].shift_func(node, stack, nametable, num_of_variables_init, err, is_init));  break;
        case OPERATOR:                        if(node->data.op >= num_of_operators) *err = INCORR_OPERATOR;
                                              CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].shift_func(node, stack, nametable, num_of_variables_init, err, is_init));      break;
        default:                              *err = INCORR_TYPE;                                                                                                                  break; 
        }
}

void CalculRpbShiftOther(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable, num_of_variables_init, err, is_init));
}

void CalculRpbShiftFuncCall(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    if(*err) return;
    node->data.var_code = nametable->first_free;

    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable, num_of_variables_init, err, false));
}

void CalculRpbShiftFuncDecl(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    if(*err) return;

    nametable = NameTableInit();
    for(size_t count = 0; count < *num_of_variables_init; count++){
        CHECK_PARSING_ERR(*err = stack_pop(stack, NULL));
    }
    *num_of_variables_init = 0;
    CALL_FUNC_AND_CHECK_ERR_FREE(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, true));
    CALL_FUNC_AND_CHECK_ERR_FREE_NAMETAB(CalculRpbShiftRecursive(node->right, stack, nametable, num_of_variables_init, err, is_init));

    for(size_t idx = 0; idx < *num_of_variables_init; idx++){
        CHECK_PARSING_ERR(*err = stack_pop(stack, NULL));
    }

    *num_of_variables_init = 0;
    NameTableDestroy(nametable);
}

void CalculRpbShiftIfElseWhile(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    if(*err) return;
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, is_init));

    size_t num_of_variables = 0;
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable, &num_of_variables, err, is_init));

    for(size_t idx = 0; idx < num_of_variables; idx++){
        CHECK_PARSING_ERR(*err = stack_pop(stack, NULL));
    }
}

void CalculRpbShiftInit(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->left, stack, nametable, num_of_variables_init, err, true));
    CALL_FUNC_AND_CHECK_ERR(CalculRpbShiftRecursive(node->right, stack, nametable,  num_of_variables_init, err, false));
}

//-----------------------------------------------------------------------------------------
// RBP shift variable

static bool FindVarAtStack(TreeNode_t* node, Stack_t* stack, name_table* nametable);

static void TreeOutputUnusedVar(TreeNode_t *node, name_table* nametable);

void CalculRpbShiftVariable(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init, TreeErr_t* err, bool is_init){
    if(*err) return;

    if(is_init){
        (*num_of_variables_init)++;
        size_t idx = NameTableAddName(nametable, node->var_func_name);
        CHECK_PARSING_ERR(*err = stack_push(stack, &idx));
        node->data.var_code = idx;
    }
    else{
        bool is_found = FindVarAtStack(node, stack, nametable);
        if(!is_found){
            *err = USE_VAR_BEFORE_INIT;
            TreeOutputUnusedVar(node, nametable);
        }
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

// ------------------------------------------------------------------------
// Output message with unused var

static TreeNode_t* FindHeadNode(TreeNode_t *node);

static void TreeOutputUnusedVarRecursive(TreeNode_t *head_node, TreeNode_t *node, name_table* nametable, bool* is_exit, size_t num_of_str, bool* is_func_param);

static void TreeOutputUnusedVar(TreeNode_t *node, name_table* nametable){
    fprintf(stderr, RED "error: " RESET "Using undeclared variable %s\n", node->var_func_name);
    fprintf(stderr, "%zu | ",  node->num_of_str);

    TreeNode_t* head_of_func = FindHeadNode(node);
    bool is_exit = false;
    bool is_param = false;

    TreeOutputUnusedVarRecursive(head_of_func, head_of_func, nametable, &is_exit, node->num_of_str, &is_param);

    fprintf(stderr, "\n\n",  node->num_of_str);
}

static TreeNode_t* FindHeadNode(TreeNode_t *node){
    while(node -> parent && (node->type != FUNCTION || node->type != FUNCTION_MAIN)){
        node = node -> parent;
    }
    return node;
}

static void TreeOutputUnusedVarRecursive(TreeNode_t *head_node, TreeNode_t *node, name_table* nametable, bool* is_exit, size_t num_of_str, bool* is_func_param){
    if(*is_exit) return;
    if(!node) return;

    static size_t amount_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
    static size_t amount_std_func = sizeof(FUNC_INFO) / sizeof(std_func_info);

    if(node->num_of_str != num_of_str){
        TreeOutputUnusedVarRecursive(head_node, node->left, nametable, is_exit, num_of_str, is_func_param);

        if(*is_func_param && node->right) fprintf(stderr, "%s",  OPERATORS_INFO[OP_COMMA].op_name_in_code);

        TreeOutputUnusedVarRecursive(head_node, node->right, nametable, is_exit, num_of_str, is_func_param);
    }
    else if((node->type == FUNCTION || node->type == FUNCTION_MAIN) && node->var_func_name != head_node -> var_func_name){
        *is_exit = true;
        return;
    }
    else if(node->type == OPERATOR && (node->data.op == OP_WHILE || node->data.op == OP_IF)){
        fprintf(stderr, "%s %s ",  OPERATORS_INFO[node->data.op].op_name_in_code, OPERATORS_INFO[OP_OPEN_BR].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->left, nametable, is_exit, num_of_str, is_func_param);
        fprintf(stderr, " %s",  OPERATORS_INFO[OP_CLOSE_BR].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->right, nametable, is_exit, num_of_str, is_func_param);
    }
    else if(node->type == OPERATOR && node->data.op < amount_op){
        TreeOutputUnusedVarRecursive(head_node, node->left, nametable, is_exit, num_of_str, is_func_param);
        fprintf(stderr, " %s ", OPERATORS_INFO[node->data.op].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->right, nametable, is_exit, num_of_str, is_func_param);
    }
    else if((node->type == FUNCTION_STANDART_NON_VOID || node->type == FUNCTION_STANDART_VOID) && node->data.stdlib_func < amount_std_func){
        *is_func_param = true;
        fprintf(stderr, "%s %s ",  FUNC_INFO[node->data.stdlib_func].func_name_in_code, OPERATORS_INFO[OP_OPEN_BR].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->left, nametable, is_exit, num_of_str, is_func_param);
        fprintf(stderr, " %s",  OPERATORS_INFO[OP_CLOSE_BR].op_name_in_code); 
        *is_func_param = false;
    }
    else if(node->type == FUNC_CALL){
        *is_func_param = true;
        fprintf(stderr, "%s %s ", node->var_func_name, OPERATORS_INFO[OP_OPEN_BR].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->left, nametable, is_exit, num_of_str, is_func_param);
        *is_func_param = false;

        fprintf(stderr, " %s",  OPERATORS_INFO[OP_CLOSE_BR].op_name_in_code); 
        TreeOutputUnusedVarRecursive(head_node, node->right, nametable, is_exit, num_of_str, is_func_param);
    }
    else if(node->type == CONST){
        fprintf(stderr, "%lg", node->data.const_value); 
    }
    else if(node->type == VARIABLE){
        if(node->var_func_name){
            fprintf(stderr, "%s", node->var_func_name); 
        }
        else{
            fprintf(stderr, "%s", nametable->var_info[node->data.var_code]); 
        }
    }

}
