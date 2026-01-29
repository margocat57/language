#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "../../debug_output/graphviz_dump.h"
#include "../../tree/tree_func.h"
#include "../../include/operators_func.h"
#include "../../include/standart_func.h"


const double EPS = 1e-15;

#define CALL_FUNC_AND_CHECK_ERR(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return;                                                         \
        } \
    }while(0)

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// DSL for calculating funt

#define RES_L *left_result
#define RES_R *right_result
#define DEF_OP(Op, Result) \
static void Calc##Op(double* result, double* left_result, double* right_result){ \
    assert(result); \
    *result = (Result); \
}

DEF_OP(Add, RES_L + RES_R);
DEF_OP(Sub, RES_L - RES_R);
DEF_OP(Mul, RES_L * RES_R);
DEF_OP(Div, (fabs(RES_R) > EPS) ? RES_L / RES_R : 0);
DEF_OP(Deg, pow(RES_L, RES_R));
DEF_OP(Eq,  RES_L == RES_R);
DEF_OP(Le,  RES_L < RES_R);
DEF_OP(Ge,  RES_L > RES_R);


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// First part of task - tree of expressions and main function for it

static void CalcTreeExpressionRecursive(TreeNode_t* node, double* result, TreeErr_t* err);

void CalcTreeExpression(TreeNode_t* node, double* result, TreeErr_t* err){
    if(*err) return;
    assert(result);
    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR( *err = TreeNodeVerify(node);));
    
    CALL_FUNC_AND_CHECK_ERR(CalcTreeExpressionRecursive(node, result, err));

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node);))
}

//--------------------------------------------------------------------------------------------
// Postfix tree calculating
// After will be verifying

static void CalcExpWithOperator(TreeNode_t *node, double* result, double* left_result, double* right_result, TreeErr_t* err);

static void CalcExpWithConst(TreeNode_t* node, double* result);

static void CalcTreeExpressionRecursive(TreeNode_t* node, double* result, TreeErr_t* err){
    if(*err) return;
    assert(result);

    double left_result = 0;
    double right_result = 0;

    if(!node) return;

    if(node->type == OPERATOR){
        CALL_FUNC_AND_CHECK_ERR(CalcTreeExpressionRecursive(node->left, &left_result, err));
        CALL_FUNC_AND_CHECK_ERR(CalcTreeExpressionRecursive(node->right, &right_result, err));
    }
    switch(node->type){
        case INCORR_VAL: *err = INCORR_TYPE; return;
        case OPERATOR: CALL_FUNC_AND_CHECK_ERR(CalcExpWithOperator(node, result, &left_result, &right_result, err)); break;
        case CONST:                            CalcExpWithConst(node, result);                                       break;
        case VARIABLE: case FUNC_CALL: case FUNCTION_MAIN: case FUNCTION:                                            break;
        default:         *err = INCORR_TYPE; return;
    }


}
//--------------------------------------------------------------------------------

static void CalcExpWithOperator(TreeNode_t* node, double* result, double* left_result, double* right_result, TreeErr_t* err){
    if(*err) return;
    assert(result); 

    switch(node->data.op){
        case OP_ADD: CalcAdd(result, left_result, right_result); break;
        case OP_SUB: CalcSub(result, left_result, right_result); break;
        case OP_DIV: CalcDiv(result, left_result, right_result); break;
        case OP_MUL: CalcMul(result, left_result, right_result); break;
        case OP_DEG: CalcDeg(result, left_result, right_result); break;
        case OP_EQ:  CalcEq(result, left_result, right_result);  break;
        case OP_GE:  CalcGe(result, left_result, right_result); break;
        case OP_LE:  CalcLe(result, left_result, right_result); break;
        default:      return;
    }
}

static void CalcExpWithConst(TreeNode_t* node, double* result){
    *result = node->data.const_value;
}

//---------------------------------------------------------------------------
// Optimization

static void TreeOptimizeConst(TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

static void TreeOptimizeNeutral(TreeNode_t **result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

static void TreeOptimizeIf0Recursive(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

static void TreeOptimizeIf1Recursive(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

void TreeOptimize(TreeNode_t **node, TreeErr_t* err){
    assert(node);
    if(*err) return;

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)

    bool is_optimized = false;
    do{
        is_optimized = false; 
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeConst(*node, &is_optimized, err));
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutral(node, *node, &is_optimized, err));
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf0Recursive(node, *node, &is_optimized, err));
        tree_dump_func(*node, __FILE__, __func__, __LINE__, "After if(0)");
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf1Recursive(node, *node, &is_optimized, err));
    }while(is_optimized);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)
}

static void TreeOptimizeConst(TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;

    assert(node); 
    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeConst(node->left, is_optimized, err));
    }
    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeConst(node->right, is_optimized, err));
    }
    if(node->left && node->right && node->left->type == CONST && node->right->type == CONST && node->type == OPERATOR){
        CALL_FUNC_AND_CHECK_ERR(CalcExpWithOperator(node, &(node->data.const_value), &(node->left->data.const_value), &(node->right->data.const_value), err));
        NodeDtor(node->left);
        NodeDtor(node->right);
        node->type = CONST;
        node->left = NULL;
        node->right = NULL;
        *is_optimized = true;
    }
}

//--------------------------------------------------------------------------
// Optimizing neutrals

#define IS_EQUAL(node, value) ((node) && (node)->type == CONST && fabs((node)->data.const_value - value) < EPS)

static void ChangeKidParrentConn(TreeNode_t** result, TreeNode_t* node_for_change, TreeNode_t* new_node, bool* is_optimized);

static void TreeOptimizeNeutralAddSub(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err);

static void TreeOptimizeNeutralMul(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err);

static void TreeOptimizeNeutralDiv(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err);

static void TreeOptimizeNeutralDeg(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err);

static void TreeOptimizeNeutral(TreeNode_t **result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;

    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutral(result, node->left, is_optimized, err));
    }
    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutral(result, node->right, is_optimized, err));
    }

    if(node->type == OPERATOR){
        switch(node->data.op){
            case INCORR:              *err = INCORR_OPERATOR; return;
            case OP_ADD: case OP_SUB: CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutralAddSub(result, node, is_optimized, err));  break;
            case OP_MUL:              CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutralMul(result, node, is_optimized, err));     break;
            case OP_DIV:              CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutralDiv(result, node, is_optimized, err));     break;
            case OP_DEG:              CALL_FUNC_AND_CHECK_ERR(TreeOptimizeNeutralDeg(result, node, is_optimized, err));     break;
            }
    }
}

static void ChangeKidParrentConn(TreeNode_t** result, TreeNode_t* node_for_change, TreeNode_t* new_node, bool* is_optimized){
    assert(new_node);

    new_node->parent = (node_for_change)->parent;
    if((node_for_change)->parent){
        if((node_for_change) == (node_for_change)->parent->left){
            (node_for_change)->parent->left = new_node;
        }
        else{
            (node_for_change)->parent->right = new_node;
        }
    }
    else{
        *result = new_node;
    }
    NodeDtor(node_for_change);
    *is_optimized = true;
}

static void TreeOptimizeNeutralAddSub(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node->left || !node->right){
        *err = NO_ELEM_FOR_BINARY_OP;
        return;
    }
    if(IS_EQUAL(node->left, 0) && node->data.op != OP_SUB){
        TreeDelNodeRecur(node->left);
        ChangeKidParrentConn(result, node, node->right, is_optimized); 
    }
    else if(IS_EQUAL(node->right, 0)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
}

static void TreeOptimizeNeutralMul(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node->left || !node->right){
        *err = NO_ELEM_FOR_BINARY_OP;
        return;
    }
    if(IS_EQUAL(node->left, 0) || IS_EQUAL(node->right, 1)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
    else if(IS_EQUAL(node->right, 0) || IS_EQUAL(node->left, 1)){
        TreeDelNodeRecur(node->left);
        ChangeKidParrentConn(result, node, node->right, is_optimized);
    }
}

static void TreeOptimizeNeutralDiv(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node->left || !node->right){
        *err = NO_ELEM_FOR_BINARY_OP;
        return;
    }
    if(IS_EQUAL(node->left, 0) && !IS_EQUAL(node->right, 0)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
    else if(IS_EQUAL(node->right, 1)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
}

static void TreeOptimizeNeutralDeg(TreeNode_t** result, TreeNode_t* node, bool* is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node->left || !node->right){
        *err = NO_ELEM_FOR_BINARY_OP;
        return;
    }
    if((IS_EQUAL(node->left, 0) && !IS_EQUAL(node->right, 0))){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
    else if(IS_EQUAL(node->left, 1)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
    else if(IS_EQUAL(node->right, 0) && !IS_EQUAL(node->left, 0)){
        node->right->data.const_value += 1;
        TreeDelNodeRecur(node->left);
        ChangeKidParrentConn(result, node, node->right, is_optimized);
    }
    else if(IS_EQUAL(node->right, 1)){
        TreeDelNodeRecur(node->right);
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
}

//-----------------------------------------------------------------------------
// Dump warnings for if optimization

static void TreeReadIf(TreeNode_t *node, bool is_if0);

//-----------------------------------------------------------------------------
// Optimizing if(0)

static void TreeOptimizeIf0(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

static void TreeOptimizeIf0Recursive(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node) return;

    if(node && node->left){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf0Recursive(result, node->left, is_optimized, err));
    }
    if(node && node->right){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf0Recursive(result, node->right, is_optimized, err));
    }

    if(node && node->type == OPERATOR && node->data.op == OP_IF && IS_EQUAL(node->left, 0)){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf0(result, node, is_optimized, err));
        return;
    }

    if(node && node->need_to_delete){
        ChangeKidParrentConn(result, node, node->right, is_optimized);
    }

}

static void TreeOptimizeIf0(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;

    TreeReadIf(node, true);

    node = node->parent;

    TreeDelNodeRecur(node->left);
    node->left = NULL;

    if(node->right && node->right->type == OPERATOR && node->right->data.op == OP_ELSE){
        ChangeKidParrentConn(result, (node->right), node->right->right, is_optimized);
    }

    node->need_to_delete = true;
}

//-----------------------------------------------------------------------------
// Optimizing if(1)

static void TreeOptimizeIf1(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err);

static void TreeOptimizeIf1Recursive(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;
    if(!node) return;

    if(node->left){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf1Recursive(result, node->left, is_optimized, err));
    }
    if(node->right){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf1Recursive(result, node->right, is_optimized, err));
    }

    if(node && node->type == OPERATOR && node->data.op == OP_IF && node->left->type == CONST && !IS_EQUAL(node->left, 0)){
        CALL_FUNC_AND_CHECK_ERR(TreeOptimizeIf1(result, node, is_optimized, err));
        return;
    }

    if(node->need_to_delete){
        ChangeKidParrentConn(result, node, node->left, is_optimized);
    }
}

static void TreeOptimizeIf1(TreeNode_t** result, TreeNode_t *node, bool *is_optimized, TreeErr_t* err){
    if(*err) return;

    TreeReadIf(node, false);

    TreeDelNodeRecur(node->left);
    node->left = NULL;
    bool is_else = false;

    if(node->parent->right && node->parent->right->type == OPERATOR && node->parent->right->data.op == OP_ELSE){
        TreeDelNodeRecur(node->parent->right);
        node->parent->right = NULL;
        is_else = true;
    }

    node = node->parent;
    ChangeKidParrentConn(result, (node->left), node->left->right, is_optimized);

    if(is_else) node->need_to_delete = true;
}

//-----------------------------------------------------------------------------
// Making warning for if(0) and if(1)

#define PURPLE_WARNING             "\033[1;35m"
#define RESET                      "\033[0m"

static void TreeReadIfRecursive(TreeNode_t *node);

static void TreeReadIf(TreeNode_t *node, bool is_if0){
    if(is_if0) fprintf(stderr, PURPLE_WARNING "warning: " RESET "This code will never be executed:\n");
    else       fprintf(stderr, PURPLE_WARNING "warning: " RESET "This code will always be executed:\n");

    fprintf(stderr, "\t %s %s",  OPERATORS_INFO[OP_IF].op_name_in_code, OPERATORS_INFO[OP_OPEN_BR].op_name_in_code); 

    TreeReadIfRecursive(node->left);

    fprintf(stderr, "%s\n",  OPERATORS_INFO[OP_CLOSE_BR].op_name_in_code); 
}

static void TreeReadIfRecursive(TreeNode_t *node){
    if(!node) return;

    static size_t num_of_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
    static size_t std_func  = sizeof(FUNC_INFO) / sizeof(std_func_info);

    TreeReadIfRecursive(node->left);

    if(node->type == OPERATOR && node->data.op < num_of_op){
        fprintf(stderr, " %s ", OPERATORS_INFO[node->data.op]);
    }
    else if(node->type == CONST){
        fprintf(stderr, " %lg ", node->data.const_value);
    }
    else if((node->type == VARIABLE || node->type == FUNCTION || node->type == FUNCTION_MAIN || node->type == FUNC_CALL) && node->var_func_name){
        fprintf(stderr, " %s ", node->var_func_name);
    }
    else if((node->type == FUNCTION_STANDART_VOID || node->type == FUNCTION_STANDART_NON_VOID ) && node->data.stdlib_func < std_func){
        fprintf(stderr, " %s ", FUNC_INFO[node->data.op]);
    }

    TreeReadIfRecursive(node->right);
}

#undef PURPLE_WARNING             
#undef RESET              

//-----------------------------------------------------------------------------
// Undef dsl
#undef IS_EQUAL
// #undef IS_NODE_OP
#undef RES_L
#undef RES_R
#undef DEF_OP
#undef CALL_FUNC_AND_CHECK_ERR