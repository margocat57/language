#ifndef MAKE_TOKENS_TREE_H
#define MAKE_TOKENS_TREE_H
#include "../include/tree.h"
#include "../common/tree_func.h"
#include "../include/mistakes.h"
#include "../common/metki.h"
#include "../debug_output/graphviz_dump.h"
#include "../include/operators_func.h"
#include "../common/tokens.h"

typedef uint64_t SyntaxErr_t;

enum SyntaxErr{
    NO_SYNTAX_ERR,
    AT_LEAST_ONE_OP_EXPREXTED,
    NO_VARIABLE_IN_INIT_FUNC,
    NO_EXPR_IN_INIT_FUNC, 
    NO_SP_AFTER_INIT_IN_INIT_FUNC, 
    NO_VARIABLE_IN_ASS_FUNC, 
    NO_EXPR_IN_ASS_FUNC, 
    NO_SP_IN_ASS_FUNC,
    NO_OPEN_BR_BEFORE_CONDITION_IN_IF_EXPR, 
    NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR,
    NO_OPEN_FIGURE_BR_BEFORE_IF, 
    NO_CLOSE_FIGURE_BR_AFTER_IF, 
    NO_OPEN_FIGURE_BR_BEFORE_ELSE, 
    NO_CLOSE_FIGURE_BR_AFTER_ELSE, 
    NO_OPEN_BR_BEFORE_CONDITION_IN_WHILE, 
    NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE,
    NO_OPEN_FIGURE_BR_BEFORE_WHILE, 
    NO_CLOSE_FIGURE_BR_AFTER_WHILE, 
    NO_CLOSE_BRACKET_IN_MATH_EXPR, 
    INCORR_OPERAND_NOT_VAR_NOT_NUM
};

TreeHead_t* MakeLangExprTokens(Tokens_t* tokens);

#endif //MAKE_TOKENS_TREE_H