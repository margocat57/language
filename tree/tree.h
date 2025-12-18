#ifndef TREE_H
#define TREE_H
#include <stdint.h>
#include <string.h>

const char *const TREE_SIGNATURE = "My_tree";
const int POISON = -6666;

enum OPERATORS
{
    INCORR,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_DEG,
    OP_INIT,
    OP_ASS,
    OP_EQ,
    OP_GE,
    OP_LE,
    OP_SP,
    OP_IF,
    OP_ELSE,
    OP_WHILE,
    OP_OPEN_BR,
    OP_CLOSE_BR,
    OP_OPEN_FIG_BR,
    OP_CLOSE_FIG_BR,
    OP_COMMA,
    OP_RETURN,
    OP_INPUT,
    OP_OUTPUT,
    OP_EXIT,
    OP_RAM_DUMP,
};

union TreeElem_t{
    enum OPERATORS op;
    double const_value;
    size_t var_code; 
};

#ifdef _DEBUG_SMALL_TREE
#define DEBUG_TREE(...) __VA_ARGS__
#else
#define DEBUG_TREE(...)
#endif //_DEBUG_SMALL_TREE

#define CHECK_AND_RET_TREEERR(bad_condition)\
    {\
    TreeErr_t err = (bad_condition);\
    if (err)                                                                          \
    {                                                                                           \
        fprintf(stderr, "err = %llu, %s, %s, %d\n", err, __FILE__, __func__, __LINE__); \
        return err;                                                                   \
    }}

enum VALUE_TYPE
{
    INCORR_VAL,
    OPERATOR,
    VARIABLE,
    FUNCTION,
    FUNCTION_MAIN,
    CONST,
    FUNC_CALL
};

struct TreeNode_t
{
    VALUE_TYPE type;
    TreeElem_t data;
    char *var_func_name;
    TreeNode_t *left;
    TreeNode_t *right;
    TreeNode_t *parent;
    const char *signature;
};

struct TreeHead_t
{
    TreeNode_t *root;
};

#endif // TREE_H