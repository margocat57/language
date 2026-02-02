#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include "make_tokens_tree.h"

// -------------------------------------------------------------------------------------
// Define lib

#define RED                        "\033[1;31m"
#define GREEN                      "\033[0;32m"
#define RESET                      "\033[0m"

#define CALL_AND_CHECK_ERR(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return NULL;                                                         \
        } \
    }while(0)

#define CHECK_PARSING_ERR(bad_condition, val)\
    do{\
    *err = (bad_condition);\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return val; \
        } \
    }while(0) \

#define FAIL_IF(bad_condition, err_code, is_in_end_of_str)\
    if(bad_condition){ \
        *err = err_code; \
        if(err_code < MAX_ERR_NUM){\
            fprintf(stderr, RED "error: " RESET "%s\n", ErrorsAndInfo[err_code].msg);\
        }\
        OutputErr(pos, tokens, is_in_end_of_str); \
        return NULL; \
    } \

//-------------------------------------------------------------------------------------
// DSL

#define IS_TYPE_IN_POS(type_)           (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == type_)
#define IS_OPERATOR_IN_POS(op_)         (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR  && tokens->node_arr[*pos]->data.op == op_)
#define IS_INICIALIZE_IN_POS           (*pos + 1 < tokens->first_free_place && tokens->node_arr[*pos + 1] && tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_INIT)
#define IS_ASSIGN_IN_POS               (*pos + 1 < tokens->first_free_place && tokens->node_arr[*pos + 1] && tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_ASS)


// -------------------------------------------------------------------------------------
// Helpful function

static TreeNode_t* GetSepNode(Tokens_t* tokens_copy, TreeNode_t* node_left_child, OPERATORS sep_op, SyntaxErr_t* err){
    if(*err) return NULL;

    CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = sep_op}, NULL, node_left_child, NULL), tokens_copy), NULL);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    if (node_left_child) node_left_child -> parent = sep_node;

    return sep_node;
}

// -------------------------------------------------------------------------------------
// Error messages verifyier

ErrAndMsg_mistake_t find_err_and_msg_mistakes(){
    for(size_t error = 0; error < NUM_OF_SYNTAX_ERR; error++){
        if(error != ErrorsAndInfo[error].err){
            fprintf(stderr, "Err %zu and err is %d - not simillar\n", error, ErrorsAndInfo[error].err);
            return INDEX_AND_ERROR_NOT_SIMMILAR;
        }
        if(error != 0 && !ErrorsAndInfo[error].msg){
            fprintf(stderr, "No message for error %zu\n", error);
            return NO_MSG_FOR_ERROR;
        }
    }
    return NO_MISTAKE_MSG_ERR;
};

// -------------------------------------------------------------------------------------
// For output errors

static int count_digits_log(size_t num);

static void OutputIncorrStr(Tokens_t* tokens, size_t curr_num_of_str, ssize_t* end_of_syntax_constr);

static void OutputUnderline(size_t* pos, Tokens_t* tokens, size_t width, size_t curr_num_of_str, ssize_t end_of_syntax_constr);

static void OutputErr(size_t* pos, Tokens_t* tokens, bool is_in_end_of_str){
    assert(pos); 
    if(!tokens || !tokens->node_arr) return;

    size_t curr_num_of_str = 0; 
    if(is_in_end_of_str && *pos != 0) curr_num_of_str = tokens->node_arr[*pos - 1]->num_of_str;
    else                              curr_num_of_str = tokens->node_arr[*pos]->num_of_str;

    int width = count_digits_log(curr_num_of_str);
    fprintf(stderr, "%*zu |", width, curr_num_of_str);

    ssize_t end_of_syntax_constr = 0;
    OutputIncorrStr(tokens, curr_num_of_str,  &end_of_syntax_constr);

    OutputUnderline(pos, tokens, width, curr_num_of_str, end_of_syntax_constr);
}

static int count_digits_log(size_t num){
    if (num == 0) return 1;
    return (int)log10(num) + 1;
}

static void OutputIncorrStr(Tokens_t* tokens, size_t curr_num_of_str, ssize_t* end_of_syntax_constr){
    assert(end_of_syntax_constr);

    for(size_t idx = 0; tokens->node_arr[idx]->num_of_str <= curr_num_of_str; idx++){
        if(!(tokens->node_arr[idx]) || tokens->node_arr[idx]->num_of_str < curr_num_of_str){
            continue;
        }

        if(tokens->node_arr[idx]->pos_in_str - (*end_of_syntax_constr) >= 0){
            fprintf(stderr, "%*s", tokens->node_arr[idx]->pos_in_str - (*end_of_syntax_constr), ""); // вывод пробелов
        } 

        if(tokens->node_arr[idx]->type == OPERATOR && tokens->node_arr[idx]->data.op < NUM_OF_OP){
            fprintf(stderr, "%s", OPERATORS_INFO[tokens->node_arr[idx]->data.op].op_name_in_code);
            (*end_of_syntax_constr) = OPERATORS_INFO[tokens->node_arr[idx]->data.op].num_of_symb_code + tokens->node_arr[idx]->pos_in_str;
        }
        else if(tokens->node_arr[idx]->type == CONST){
            fprintf(stderr, "%lg", tokens->node_arr[idx]->data.const_value);
            (*end_of_syntax_constr) = tokens->node_arr[idx]->pos_in_str + tokens->node_arr[idx]->num_of_symb_double;
        }
        else if((tokens->node_arr[idx]->type == VARIABLE || tokens->node_arr[idx]->type == FUNCTION || tokens->node_arr[idx]->type == FUNCTION_MAIN || tokens->node_arr[idx]->type == FUNC_CALL) && tokens->node_arr[idx]->var_func_name){
            fprintf(stderr, "%s", tokens->node_arr[idx]->var_func_name);
            (*end_of_syntax_constr) = tokens->node_arr[idx]->pos_in_str + strlen(tokens->node_arr[idx]->var_func_name);
        }
        else if((tokens->node_arr[idx]->type == FUNCTION_STANDART_VOID || tokens->node_arr[idx]->type == FUNCTION_STANDART_NON_VOID ) && tokens->node_arr[idx]->data.stdlib_func < NUM_OF_STD_FUNC){
            fprintf(stderr, "%s", FUNC_INFO[tokens->node_arr[idx]->data.op].func_name_in_code);
            (*end_of_syntax_constr) = FUNC_INFO[tokens->node_arr[idx]->data.op].num_of_symb_code + tokens->node_arr[idx]->pos_in_str;
        }
    }

    fprintf(stderr, "\n");
}

static void OutputUnderline(size_t* pos, Tokens_t* tokens, size_t width, size_t curr_num_of_str, ssize_t end_of_syntax_constr){
    fprintf(stderr, "%*s |", width, ""); 
    if(curr_num_of_str == tokens->node_arr[*pos]->num_of_str){
        fprintf(stderr, "%*s" GREEN "^" RESET"\n", tokens->node_arr[*pos]->pos_in_str, "");
    }
    else{
        fprintf(stderr, "%*s" GREEN "^" RESET"\n", tokens->node_arr[*pos]->pos_in_str + end_of_syntax_constr, "");
    }

    fprintf(stderr, "\n");
}

// -------------------------------------------------------------------------------------

/*
    !ПОКА НЕ ДОБАВЛЕНА СТАНДАРТНАЯ НЕ VOID ФУНКЦИЯ то пока соотв парсера нет
    G     ::= X
    X     ::= FUNCTION+
    FUNCTION := FUNC_DECL BODY
    FUNC_DECL ::= "func_name" (E {, E}*)+ 
    FUNC_USE ::= "func_name" (E {, E}*)+ 
    BODY ::= '{'STATEMENT+'}'
    STATEMENT :=  INIT; |  A; | IF | WHILE | RETURN; | OUTPUT; | FUNC_CALL_AFTER_INIT(без точки с запятой) | FUNC_STANDART_VOID_CALL | EXIT | RAM_DUMP | Break(только для while);
    INIT  ::= V ':=' E  | INPUT; 
    A     ::= V  '=' E  | INPUT;
    IF ::= "if" "(" E ")" STATEMENT | BODY ("else" STATEMENT | BODY)?
    WHILE  ::= "while" "(" E ")" STATEMENT | BODY
    RETURN ::= "return" E ;
    INPUT  ::= "input"  E ;
    OUTPUT ::= "output" E ;
    EXIT   ::= "exit";
    E     ::= M{[&&, ||]M}*
    M     ::= L{[>, == ,<, <=, >=]L}*
    L     ::= T{[+, -]T}* 
    T     ::= D{[*, \]D}*
    D     ::= P{[^] P}* 
    P     ::= '(' E ')' | N | V | FUNC_CALL_AFTER_INIT
    N     ::= ['0' - '9']+
    V     ::= ['a' - 'z', '_']+['a' - 'z', '0' - '9', '_']*
*/

static TreeNode_t* GetG         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy);
static TreeNode_t* GetX         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetFUNC      (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetFUNC_DECL (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetFUNC_USE  (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetBODY      (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err, bool is_while);
static TreeNode_t* GetSTATEMENT (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err, bool is_while);
static TreeNode_t* GetINIT      (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetA         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetIF        (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetWHILE     (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetRETURN    (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetINPUT     (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);
static TreeNode_t* GetOUTPUT    (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetBreak     (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);
static TreeNode_t* GetEXIT      (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);
static TreeNode_t* GetRAM_DUMP  (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);
static TreeNode_t* GetE         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetL         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetT         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetD         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetP         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);
static TreeNode_t* GetN         (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);
static TreeNode_t* GetV         (size_t* pos, Tokens_t* tokens,                        SyntaxErr_t* err);

// -------------------------------------------------------------------------------------

TreeHead_t* MakeLangExprTokens(Tokens_t* tokens, Tokens_t* tokens_copy){
    DEBUG_TREE(
    ErrAndMsg_mistake_t error = find_err_and_msg_mistakes();
    if(error){
        fprintf(stderr, "error %zu in synatx err table", error);
        return NULL;
    }
    )

    TreeHead_t* head = TreeCtor();
    size_t pos = 0;
    head->root = GetG(&pos, tokens, tokens_copy);
    if(!head->root){
        return NULL;
    }
    DEBUG_TREE(
    if(TreeVerify(head)){
        fprintf(stderr, "File is not correct - can't work with created tree\n");
        TokensDtor(tokens);
        TokensDtor(tokens_copy);
        free(head);
        return NULL;
    }
    )
    return head;
}

// -------------------------------------------------------------------------------------
// GetG

static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy){
    SyntaxErr_t err = NO_MISTAKE;
    TreeNode_t* first_op = GetX(pos, tokens, tokens_copy, &err);

    if(err || *pos != tokens->first_free_place){
        if(err) fprintf(stderr, "Syntax error %llu\n", err);
        else    fprintf(stderr, "Position not equal to first free %llu\n", err);
        TokensDtor(tokens);
        TokensDtor(tokens_copy);
        return NULL;
    }
    assert(first_op);
    tree_dump_func(first_op, __FILE__, __func__, __LINE__ , "Before ret GetG node %zu", *pos);
    return first_op;
}

// -------------------------------------------------------------------------------------
// GetX and helping functions

static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    assert(err);
    if(*err) return NULL;

    TreeNode_t* node_general = NULL;
    CALL_AND_CHECK_ERR(node_general = GetFUNC(pos, tokens, tokens_copy, err));

    TreeNode_t* node_left = node_general;

    while(IS_TYPE_IN_POS(FUNCTION) || IS_TYPE_IN_POS(FUNCTION_MAIN)){
        TreeNode_t* node_right = NULL;
        CALL_AND_CHECK_ERR(node_right = GetFUNC(pos, tokens, tokens_copy, err));

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }
    
    assert(node_general);
    // tree_dump_func(node_general, __FILE__, __func__, __LINE__, "Before ret GetX node %zu", *pos);
    return node_general;
}

// -------------------------------------------------------------------------------------

static TreeNode_t* GetFUNC(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func_name = NULL;
    CALL_AND_CHECK_ERR(func_name = GetFUNC_DECL(pos, tokens, tokens_copy, err));

    TreeNode_t* bodynode = NULL;
    CALL_AND_CHECK_ERR(bodynode = GetBODY(pos, tokens, tokens_copy, err, false));

    bodynode -> parent = func_name;
    func_name -> right = bodynode;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(tokens_copy, func_name, OP_CLOSE_FIG_BR, err));


    // tree_dump_func(sep_node, __FILE__, __func__, __LINE__, "Before ret GetFUNC node %zu", *pos);

    return sep_node;
}

// -------------------------------------------------------------------------------------
// GetFuncDecl and GetFuncUse and helping functions

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, bool is_declaration, SyntaxErr_t* err);

static TreeNode_t* GetFUNC_DECL(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func = NULL;
    CALL_AND_CHECK_ERR(func = GetFUNC_CALL(pos, tokens, tokens_copy, true, err));
    return func;
}

static TreeNode_t* GetFUNC_USE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func = NULL;
    CALL_AND_CHECK_ERR(func = GetFUNC_CALL(pos, tokens, tokens_copy, false, err));
    return func;
}

// GetFuncCall and helping functions -------------------------------

static void ProcessComma(TreeNode_t* prev_param, TreeNode_t* current_param, TreeNode_t* comma);

static TreeNode_t* GetFUNC_STD_VOID_CALL(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!(IS_TYPE_IN_POS(FUNCTION_STANDART_VOID)), 
        NO_FUNC_NAME, 
        false)

    size_t number_of_standart_func = tokens->node_arr[*pos]->data.stdlib_func;
    FAIL_IF(number_of_standart_func >= NUM_OF_STD_FUNC, 
        NUMBER_OF_STD_FUNC_OUT_OF_ARR, 
        false)

    TreeNode_t* func_name = tokens->node_arr[*pos];
    (*pos)++; // skip FUNC_NAME
    size_t standart_func_param_count = 0;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OPEN_BR),
            NO_OPEN_BR_BEFORE_CALL_FUNC, 
            false)
    (*pos)++; // skip (

    TreeNode_t* prev_param = func_name;

    while(true){
        if(IS_OPERATOR_IN_POS(OP_CLOSE_BR)){
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* current_param = NULL;
        CALL_AND_CHECK_ERR(current_param = GetE(pos, tokens, tokens_copy, err));

        FAIL_IF(!(IS_OPERATOR_IN_POS(OP_CLOSE_BR) || IS_OPERATOR_IN_POS(OP_COMMA)),
            NO_CLOSE_BR_OR_COMMA_AFTER_PARAM, 
            true)

        if(IS_OPERATOR_IN_POS(OP_CLOSE_BR)){
            // Добавить запятую
            TreeNode_t* comma = GetSepNode(tokens_copy, NULL, OP_OPEN_BR, err);
            ProcessComma(prev_param, current_param, comma);
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* comma = GetSepNode(tokens_copy, NULL, OP_OPEN_BR, err);
        ProcessComma(prev_param, current_param, comma);

        (*pos)++; // skip comma
        prev_param = comma;
        standart_func_param_count++;
    }

    FAIL_IF(!(standart_func_param_count <= FUNC_INFO[number_of_standart_func].num_of_params),
            INCORRECT_NUM_OF_PARAMS_FOR_STD_FUNC, 
            false)

    FAIL_IF(!(IS_OPERATOR_IN_POS(OP_SP)),
            NO_SP_AFTER_CALLING_FUNC, 
            true)
    (*pos)++;

    // tree_dump_func(func_name, __FILE__, __func__, __LINE__, "Before ret GetFUNCCALL node %zu", *pos);
    return func_name;
}

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, bool is_declaration, SyntaxErr_t* err){
    if(*err) return NULL;

    if(is_declaration){
        FAIL_IF(!(IS_TYPE_IN_POS(FUNCTION) || IS_TYPE_IN_POS(FUNCTION_MAIN)), 
            NO_FUNC_NAME, 
            false)
    }
    else{
        FAIL_IF(!(IS_TYPE_IN_POS(FUNCTION)), 
            NO_FUNC_NAME, 
            false)
    }

    TreeNode_t* func_name = tokens->node_arr[*pos];
    (*pos)++; // skip FUNC_NAME

    if(!is_declaration){
        func_name->type = FUNC_CALL;
    }

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OPEN_BR),
            NO_OPEN_BR_BEFORE_CALL_FUNC, 
            false)
    (*pos)++; // skip (

    TreeNode_t* prev_param = func_name;

    while(true){
        if(IS_OPERATOR_IN_POS(OP_CLOSE_BR)){
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* current_param = NULL;
        CALL_AND_CHECK_ERR(current_param = GetE(pos, tokens, tokens_copy, err));

        FAIL_IF(!(IS_OPERATOR_IN_POS(OP_CLOSE_BR) || IS_OPERATOR_IN_POS(OP_COMMA)),
            NO_CLOSE_BR_OR_COMMA_AFTER_PARAM, 
            true)

        if(IS_OPERATOR_IN_POS(OP_CLOSE_BR)){
            // Добавить запятую
            TreeNode_t* comma = GetSepNode(tokens_copy, NULL, OP_COMMA, err);
            ProcessComma(prev_param, current_param, comma);
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* comma = tokens->node_arr[*pos];
        ProcessComma(prev_param, current_param, comma);

        (*pos)++; // skip comma
        prev_param = comma;
    }

    assert(func_name);
    // tree_dump_func(func_name, __FILE__, __func__, __LINE__, "Before ret GetFUNCCALL node %zu", *pos);
    return func_name;
}

static void ProcessComma(TreeNode_t* prev_param, TreeNode_t* current_param, TreeNode_t* comma){
    if(!prev_param->left){
        prev_param->left = comma;
    }
    else{
        prev_param->right = comma;
    }
    comma->parent = prev_param;
    comma->left = current_param;
    current_param -> parent = comma;
}

// -------------------------------------------------------------------------------------
// GetBody

static TreeNode_t* GetBODY(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err, bool is_while){
    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OPEN_FIG_BR), 
            NO_OPEN_FIG_BR_BEFORE_STATEMENT, 
            false)
    (*pos)++; //skip {

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetSTATEMENT(pos, tokens, tokens_copy, err, is_while));

    TreeNode_t* node_left = node;
    TreeNode_t* node_right = NULL;

    while(true){
        if(IS_OPERATOR_IN_POS(OP_CLOSE_FIG_BR)){
            (*pos)++; // skip }
            break;
        }

        CALL_AND_CHECK_ERR(node_right = GetSTATEMENT(pos, tokens, tokens_copy, err, is_while));

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }

    return node;
}

// -------------------------------------------------------------------------------------
// GetSTATEMENT

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err, bool is_while){
    if(*err) return NULL;

    TreeNode_t* node = NULL;
    if(IS_INICIALIZE_IN_POS){
        CALL_AND_CHECK_ERR(node = GetINIT(pos, tokens, tokens_copy, err));
    }
    else if(IS_ASSIGN_IN_POS){
        CALL_AND_CHECK_ERR(node = GetA(pos, tokens, tokens_copy, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_IF)){
        CALL_AND_CHECK_ERR(node = GetIF(pos, tokens, tokens_copy, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_WHILE)){
        CALL_AND_CHECK_ERR(node = GetWHILE(pos, tokens,tokens_copy, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_RETURN)){
        CALL_AND_CHECK_ERR(node = GetRETURN(pos, tokens, tokens_copy, err));
    }
    else if(IS_TYPE_IN_POS(FUNCTION)){
        CALL_AND_CHECK_ERR(node = GetFUNC_USE(pos, tokens, tokens_copy, err));
        
        TreeNode_t* sep_point = GetSepNode(tokens_copy, node, OP_CLOSE_FIG_BR, err);
        node = sep_point;
    }
    else if(IS_TYPE_IN_POS(FUNCTION_STANDART_VOID)){
        CALL_AND_CHECK_ERR(node = GetFUNC_STD_VOID_CALL(pos, tokens, tokens_copy, err));

        TreeNode_t* sep_point = GetSepNode(tokens_copy, node, OP_CLOSE_FIG_BR, err);
        node = sep_point;
    }
    else if(IS_OPERATOR_IN_POS(OP_OUTPUT)){
        CALL_AND_CHECK_ERR(node = GetOUTPUT(pos, tokens, tokens_copy, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_EXIT)){
        CALL_AND_CHECK_ERR(node = GetEXIT(pos, tokens, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_RAM_DUMP)){
        CALL_AND_CHECK_ERR(node = GetRAM_DUMP(pos, tokens, err));
    }
    else if(IS_OPERATOR_IN_POS(OP_BREAK)){
        FAIL_IF(!is_while, 
                USING_BREAK_WITHOUT_WHILE, 
                false)
        CALL_AND_CHECK_ERR(node = GetBreak(pos, tokens, err));
    }
    else{
        FAIL_IF(true, 
            INCORR_STATEMENT, 
            false)
    }
    return node;
}

// -------------------------------------------------------------------------------------
// GetInit or GetA

static TreeNode_t* GetInitOrAssSubtree(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_INICIALIZE_IN_POS, 
            NO_INIT_OR_ASS_OP, 
            false)

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetInitOrAssSubtree(pos, tokens, tokens_copy, err));

    return node;
}

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_ASSIGN_IN_POS, 
            NO_INIT_OR_ASS_OP, 
            false)

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetInitOrAssSubtree(pos, tokens, tokens_copy, err));

    return node;
}

static TreeNode_t* GetInitOrAssSubtree(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node_left = NULL;
    CALL_AND_CHECK_ERR(node_left = GetV(pos, tokens, err));
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INIT or OP_ASS

    TreeNode_t* node_right = NULL;
    if(IS_OPERATOR_IN_POS(OP_INPUT)){
        CALL_AND_CHECK_ERR(node_right = GetINPUT(pos, tokens, err));
    }
    else{
        CALL_AND_CHECK_ERR(node_right = GetE(pos, tokens, tokens_copy, err));
    }

    node->left = node_left;
    node->right = node_right;
    node_left->parent = node;
    node_right->parent = node;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_SP), 
            NO_SP_AFTER_INIT_IN_INIT_FUNC, 
            true)

    TreeNode_t* node_sp = tokens->node_arr[*pos];
    node_sp -> left = node;
    node ->parent = node_sp;

    (*pos)++; // skip ;

    return node_sp;
}

// -------------------------------------------------------------------------------------
// GetIf

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_IF), 
            NO_OP_IF_IN_IF_STAT, 
            false)

    TreeNode_t* if_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip if

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OPEN_BR), 
            NO_OPEN_BR_BEFORE_CONDITION_IN_IF_EXPR, 
            false)
    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_AND_CHECK_ERR(exprnode = GetE(pos, tokens, tokens_copy, err));

    if_node -> left = exprnode;
    exprnode -> parent = if_node;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_CLOSE_BR), 
            NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR, 
            true)
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;
    if(IS_OPERATOR_IN_POS(OP_OPEN_FIG_BR)){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, err, false));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err, false));
    }

    if_node -> right = statnode;
    statnode -> parent = if_node;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(tokens_copy, if_node, OP_CLOSE_FIG_BR, err));

    // else
    TreeNode_t* opnode_else = NULL;
    if(IS_OPERATOR_IN_POS(OP_ELSE)){
        CALL_AND_CHECK_ERR(opnode_else = GetELSE(pos, tokens, tokens_copy, err));

        sep_node -> right = opnode_else;
        opnode_else ->parent = sep_node;

        TreeNode_t* sep_node_itog = NULL;
        CALL_AND_CHECK_ERR(sep_node_itog = GetSepNode(tokens_copy, sep_node, OP_CLOSE_FIG_BR, err));

        sep_node = sep_node_itog;
    }
    // ---

    // tree_dump_func(sep_node, __FILE__, __func__, __LINE__, "Before ret GetIF node %zu", *pos);
    return sep_node;
}

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_ELSE), 
            NO_OP_ELSE_IN_ELSE_STAT, 
            false)

    TreeNode_t* else_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip else

    TreeNode_t* statnode = NULL;
    if(IS_OPERATOR_IN_POS(OP_OPEN_FIG_BR)){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, err, false));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err, false));
    }

    else_node -> right = statnode;
    statnode -> parent = else_node;

    // tree_dump_func(else_node, __FILE__, __func__, __LINE__, "Before ret GetELSE node %zu", *pos);

    return else_node;
}

// -------------------------------------------------------------------------------------
// GetWhile

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_WHILE), 
            NO_OP_WHILE_IN_WHILE_STAT, 
            false)

    TreeNode_t* while_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip while

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OPEN_BR), 
            NO_OPEN_BR_BEFORE_CONDITION_IN_WHILE, 
            false)
    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_AND_CHECK_ERR(exprnode = GetE(pos, tokens, tokens_copy, err));

    while_node -> left = exprnode;
    exprnode -> parent = while_node;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_CLOSE_BR), 
            NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE, 
            true)
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;

    if(IS_OPERATOR_IN_POS(OP_OPEN_FIG_BR)){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, err, true));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err, true));
    }

    while_node -> right = statnode;
    statnode -> parent = while_node;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(tokens_copy, while_node, OP_CLOSE_FIG_BR, err));

    // tree_dump_func(sep_node, __FILE__, __func__, __LINE__, "Before ret GetWHILE node %zu", *pos);

    return sep_node;
}

// -------------------------------------------------------------------------------------
// GetReturn , Input or output

static TreeNode_t* GetRETURN(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_RETURN), 
            NO_RETURN_OP, 
            false)

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_RET

    TreeNode_t* node_e = NULL;
    CALL_AND_CHECK_ERR(node_e = GetE(pos, tokens, tokens_copy, err));

    node ->left = node_e;
    node_e -> parent = node;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_SP), 
            NO_SP_AFT_RET_INP_OUTPUT, 
            true)
    (*pos)++; // skip OP_SP - не связываем потому что после return в блоке кода ничего идти не должно

    return node;
}

static TreeNode_t* GetINPUT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_INPUT), 
            NO_INPUT_OP, 
            false)

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INPUT

    return node;
}

static TreeNode_t* GetOUTPUT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_OUTPUT), 
            NO_OUTPUT_OP, 
            true)

    TreeNode_t* node_out = tokens->node_arr[*pos];
    (*pos)++; // skip OP_RET

    TreeNode_t* node_e = NULL;
    CALL_AND_CHECK_ERR(node_e = GetE(pos, tokens, tokens_copy, err));

    node_out ->left = node_e;
    node_e -> parent = node_out;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_SP), 
            NO_SP_AFT_RET_INP_OUTPUT, 
            true)

    TreeNode_t* node_sp = tokens->node_arr[*pos];
    node_sp -> left = node_out;
    node_out-> parent = node_sp;
    (*pos)++; // skip OP_SP 

    return node_sp;
}

// ------------------------------------------------------------------------------------

static TreeNode_t* GetBreak(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_BREAK), 
            NO_BREAK_OP, 
            false)

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_BREAK

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_SP), 
            NO_SP_AFT_RET_INP_OUTPUT, 
            true)
    (*pos)++; // skip OP_SP - не связываем потому что после break в блоке кода ничего идти не должно

    return node;
}

//-------------------------------------------------------------------------------------
// GetExit

static TreeNode_t* GetEXIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_EXIT), 
            NO_RETURN_OP, 
            false)

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_EXIT

    return node;
}

//----------------------------------------------------------------------------
// Get RAM_DUMP

static TreeNode_t* GetRAM_DUMP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_OPERATOR_IN_POS(OP_RAM_DUMP), 
            NO_RETURN_OP, 
            false)

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip RAM_DUMP

    return node;
}

// -------------------------------------------------------------------------------------
// GetE

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetL(pos, tokens, tokens_copy, err)); 

    while(IS_OPERATOR_IN_POS(OP_GE)|| IS_OPERATOR_IN_POS(OP_LE) || IS_OPERATOR_IN_POS(OP_EQ) || IS_OPERATOR_IN_POS(OP_GEQ)|| IS_OPERATOR_IN_POS(OP_LEQ)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip < or > or == or <= or >=

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetL(pos, tokens, tokens_copy, err));

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    // tree_dump_func(left, __FILE__, __func__, __LINE__, "Before ret GetE node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetL

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetT(pos, tokens, tokens_copy, err)); 

    while(IS_OPERATOR_IN_POS(OP_ADD) || IS_OPERATOR_IN_POS(OP_SUB)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetT(pos, tokens, tokens_copy, err));

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    // tree_dump_func(left, __FILE__, __func__, __LINE__, "Before ret GetE node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetT

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetD(pos, tokens, tokens_copy, err)); 

    while(IS_OPERATOR_IN_POS(OP_MUL) || IS_OPERATOR_IN_POS(OP_DIV)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip * or /

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetD(pos, tokens, tokens_copy, err)); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    // tree_dump_func(left, __FILE__, __func__, __LINE__, "Before ret GetT node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetD

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetP(pos, tokens, tokens_copy, err)); 

    while(IS_OPERATOR_IN_POS(OP_DEG)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip ^

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetP(pos, tokens, tokens_copy, err)); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    // tree_dump_func(left, __FILE__, __func__, __LINE__, "Before ret GetD node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetP

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;
    TreeNode_t* val = NULL;

    if(IS_OPERATOR_IN_POS(OP_OPEN_BR)){
        (*pos)++; // skip '('

        CALL_AND_CHECK_ERR(val = GetE(pos, tokens, tokens_copy, err));

        FAIL_IF(!IS_OPERATOR_IN_POS(OP_CLOSE_BR), 
                NO_CLOSE_BRACKET_IN_MATH_EXPR, 
                true)
        (*pos)++; // skip ')'
    }
    else{
        if(IS_TYPE_IN_POS(CONST)){
            CALL_AND_CHECK_ERR(val = GetN(pos, tokens, err));
            (*pos)++;
        }
        else if(IS_TYPE_IN_POS(VARIABLE)){
            CALL_AND_CHECK_ERR(val = GetV(pos, tokens, err));
            (*pos)++;
        }
        else if(IS_TYPE_IN_POS(FUNCTION)){
            CALL_AND_CHECK_ERR(val = GetFUNC_USE(pos, tokens, tokens_copy,  err));
        }
        else{
            FAIL_IF(true, 
                    INCORR_OPERAND_NOT_VAR_NOT_NUM, 
                    false)
        } 
    }
    // tree_dump_func(val, __FILE__, __func__, __LINE__, "Before ret GetP node %zu", *pos);
    return val;
}

// -------------------------------------------------------------------------------------
// GetN

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    FAIL_IF(!IS_TYPE_IN_POS(CONST), 
            NO_CONST, 
            false)

    return tokens->node_arr[*pos];
}

// -------------------------------------------------------------------------------------
// GetV and hrlping func

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    FAIL_IF(!IS_TYPE_IN_POS(VARIABLE), 
            NO_VARIABLE, 
            false)

    FAIL_IF(!tokens->node_arr[*pos]->var_func_name,
        EMPTY_VAR_NAME,
        false);

    return tokens->node_arr[*pos];
}

//----------------------------------------------------------------------
// UNDEF DSL

#undef IS_TYPE_IN_POS 
#undef IS_OPERATOR_IN_POS  
#undef IS_INICIALIZE_IN_POS   
#undef IS_ASSIGN_IN_POS   
#undef RED
#undef GREEN           
#undef RESET  