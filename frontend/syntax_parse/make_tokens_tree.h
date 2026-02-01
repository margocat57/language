#ifndef MAKE_TOKENS_TREE_H
#define MAKE_TOKENS_TREE_H
#include "../../tree/tree.h"
#include "../../tree/tree_func.h"
#include "../../tree/mistakes.h"
#include "../../debug_output/graphviz_dump.h"
#include "../../include/operators_func.h"
#include "../../include/standart_func.h"
#include "../common/tokens.h"

typedef uint64_t SyntaxErr_t;

enum SyntaxErr{
    NO_SYNTAX_ERR,
    NO_SP_AFTER_INIT_IN_INIT_FUNC, 
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
    INCORR_OPERAND_NOT_VAR_NOT_NUM,
    NO_OPEN_BR_BEFORE_CALL_FUNC,
    NO_CLOSE_BR_OR_COMMA_AFTER_PARAM,
    NO_OPEN_BR_BEF_CALL_FUNC,
    NO_CLOSE_BR_AFT_CALL_FUNC,
    NO_OPEN_FIG_BR_BEFORE_STATEMENT,
    NO_CLOSE_FIG_BR_AFTER_STATEMENT,
    NO_SP_AFT_RET_INP_OUTPUT,
    NO_FUNC_NAME,
    NO_RETURN_OP,
    NO_INIT_OR_ASS_OP,
    NO_OP_IF_IN_IF_STAT,
    NO_OP_ELSE_IN_ELSE_STAT,
    NO_OP_WHILE_IN_WHILE_STAT,
    NO_CONST,
    NO_VARIABLE,
    INCORR_STATEMENT,
    EMPTY_VAR_NAME,
    INCORRECT_NUM_OF_PARAMS_FOR_STD_FUNC,
    NUMBER_OF_STD_FUNC_OUT_OF_ARR,
    NO_CLOSE_BR_AFTER_CALL_FUNC,
    NO_SP_AFTER_CALLING_FUNC,
    NO_INPUT_OP,
    NO_OUTPUT_OP,
    NO_BREAK_OP,
    NO_SP_AFTER_BREAK,
    USING_BREAK_WITHOUT_WHILE,
};

struct ErrAndMsg{
    SyntaxErr err;
    const char* msg;
};

const ErrAndMsg ErrorsAndInfo[] = {
    {NO_SYNTAX_ERR},
    {NO_SP_AFTER_INIT_IN_INIT_FUNC, "porco_dio! expected"}, 
    {NO_OPEN_BR_BEFORE_CONDITION_IN_IF_EXPR, "(Ciucciami expected"}, 
    {NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR, "il_cazzo!) expected"},
    {NO_OPEN_FIGURE_BR_BEFORE_IF, "{Voi-Figlio expected"}, 
    {NO_CLOSE_FIGURE_BR_AFTER_IF, "di_putana} expected"}, 
    {NO_OPEN_FIGURE_BR_BEFORE_ELSE, "{Voi-Figlio expected"}, 
    {NO_CLOSE_FIGURE_BR_AFTER_ELSE, "di_putana} expected"}, 
    {NO_OPEN_BR_BEFORE_CONDITION_IN_WHILE, "(Ciucciami expected"}, 
    {NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE, "il_cazzo!) expected"},
    {NO_OPEN_FIGURE_BR_BEFORE_WHILE, "{Voi-Figlio expected"},
    {NO_CLOSE_FIGURE_BR_AFTER_WHILE, "di_putana} expected"},  
    {NO_CLOSE_BRACKET_IN_MATH_EXPR, "il_cazzo!) expected"},
    {INCORR_OPERAND_NOT_VAR_NOT_NUM, "incorrect operand - function, variable or number expected"},
    {NO_OPEN_BR_BEFORE_CALL_FUNC, "(Ciucciami expected"}, 
    {NO_CLOSE_BR_OR_COMMA_AFTER_PARAM, "il_cazzo!) or сhe_culo expected"},
    {NO_OPEN_BR_BEF_CALL_FUNC, "(Ciucciami expected"}, 
    {NO_CLOSE_BR_AFT_CALL_FUNC, "il_cazzo!) expected"}, 
    {NO_OPEN_FIG_BR_BEFORE_STATEMENT, "{Voi-Figlio expected"}, 
    {NO_CLOSE_FIG_BR_AFTER_STATEMENT, "di_putana} expected"}, 
    {NO_SP_AFT_RET_INP_OUTPUT, "porco_dio! expected"}, 
    {NO_FUNC_NAME, "can't get function name"},
    {NO_RETURN_OP, "succhia expected"},
    {NO_INIT_OR_ASS_OP, "fare_pompino or a_fanculo expected"},
    {NO_OP_IF_IN_IF_STAT, "Culo expected"},
    {NO_OP_ELSE_IN_ELSE_STAT, "Fica expected"},
    {NO_OP_WHILE_IN_WHILE_STAT, "Fottere expected"},
    {NO_CONST, "no const value"},
    {NO_VARIABLE, "no variable"},
    {INCORR_STATEMENT, "invalid operands"},
    {EMPTY_VAR_NAME, "empty variable name"},
    {INCORRECT_NUM_OF_PARAMS_FOR_STD_FUNC, "too few or many arguments for standart function"},
    {NUMBER_OF_STD_FUNC_OUT_OF_ARR, "implicit declaration of function"},
    {NO_CLOSE_BR_AFTER_CALL_FUNC, "il_cazzo!) expected"}, 
    {NO_SP_AFTER_CALLING_FUNC, "porco_dio! expected"}, 
    {NO_INPUT_OP, "Infila_сazzo expected"},
    {NO_OUTPUT_OP, "Estrarre_cazzo expected"},
    {NO_BREAK_OP, "Vattone! expected"},
    {NO_SP_AFTER_BREAK, "porco_dio! expected"}, 
    {USING_BREAK_WITHOUT_WHILE, "Vattone! statement not in loop"}
};

const size_t NUM_OF_SYNTAX_ERR = sizeof(ErrorsAndInfo) / sizeof(ErrAndMsg);

typedef uint64_t ErrAndMsg_mistake_t;

enum MESSAGES_AND_ERR_MISTAKES{
    NO_MISTAKE_MSG_ERR,
    INDEX_AND_ERROR_NOT_SIMMILAR,
    NO_MSG_FOR_ERROR
};


const size_t MAX_ERR_NUM = sizeof(ErrorsAndInfo) / sizeof(ErrAndMsg);

TreeHead_t* MakeLangExprTokens(Tokens_t* tokens, Tokens_t* tokens_copy);

#endif //MAKE_TOKENS_TREE_H