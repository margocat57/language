#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "../stack_frontend/stack.h"
#include "../stack_frontend/stack_func.h"
#include "make_tokens_tree.h"

const size_t MAX_BUF_SIZE = 2048;

// -------------------------------------------------------------------------------------
// Define lib

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

#define FAIL_IF(bad_condition, err_code, msg)\
    if(bad_condition){ \
        *err = err_code; \
        fprintf(stderr, msg); \
        fprintf(stderr, "\n%s %s %d\n", __FILE__, __func__, __LINE__); \
        return NULL; \
    } \

//-------------------------------------------------------------------------------------
// DSL

#define IS_FUNCTION_IN_POS             (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == FUNCTION)
#define IS_VARIABLE_IN_POS             (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == VARIABLE)
#define IS_CONST_IN_POS                (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == CONST)
#define IS_OPEN_BRACKET_IN_POS         (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_OPEN_BR)
#define IS_CLOSE_BRACKET_IN_POS        (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)
#define IS_OPEN_FIGURE_BRACKET_IN_POS  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)
#define IS_CLOSE_FIGURE_BRACKET_IN_POS (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)
#define IS_IF_IN_POS                   (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_IF)
#define IS_ELSE_IN_POS                 (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_ELSE)
#define IS_WHILE_IN_POS                (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_WHILE)
#define IS_RETURN_IN_POS               (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_RETURN)
#define IS_SEPARATION_POINT_IN_POS     (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_SP)
#define IS_COMMA_IN_POS                (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_COMMA)
#define IS_GE_IN_POS                   (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_GE)
#define IS_LE_IN_POS                   (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_LE)
#define IS_EQ_IN_POS                   (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_EQ)
#define IS_ADD_IN_POS                  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_ADD)
#define IS_SUB_IN_POS                  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_SUB)
#define IS_MUL_IN_POS                  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_MUL)
#define IS_DIV_IN_POS                  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_DIV)
#define IS_DEG_IN_POS                  (*pos < tokens->first_free_place     && tokens->node_arr[*pos]     && tokens->node_arr[*pos]->type == OPERATOR     && tokens->node_arr[*pos]->data.op == OP_DEG)
#define IS_INICIALIZE_IN_POS           (*pos + 1 < tokens->first_free_place && tokens->node_arr[*pos + 1] && tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_INIT)
#define IS_ASSIGN_IN_POS               (*pos + 1 < tokens->first_free_place && tokens->node_arr[*pos + 1] && tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_ASS)

// -------------------------------------------------------------------------------------
// Helpful function

static TreeNode_t* GetSepNode(size_t* pos, Tokens_t* tokens_copy, TreeNode_t* node_left_child, OPERATORS sep_op, SyntaxErr_t* err){
    if(*err) return NULL;

    CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = sep_op}, NULL, node_left_child, NULL), tokens_copy), NULL);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    node_left_child -> parent = sep_node;

    return sep_node;
}

// -------------------------------------------------------------------------------------

/*
    G     ::= X
    X     ::= FUNCTION+
    FUNCTION := FUNC_DECL BODY
    FUNC_DECL ::= "func_name" (E {, E}*)+ 
    FUNC_USE ::= "func_name" (E {, E}*)+ 
    BODY ::= '{'STATEMENT+'}'
    STATEMENT :=  INIT; |  A; | IF | WHILE | RETURN; | FUNC_CALL_AFTER_INIT;
    INIT  ::= V ':=' E; 
    A     ::= V '=' E;
    IF ::= "if" "(" E ")" STATEMENT | BODY ("else" STATEMENT | BODY)?
    WHILE  ::= "while" "(" E ")" STATEMENT | BODY
    RETURN ::= "return" E ;
    E     ::= M{[&&, ||]M}*
    M     ::= L{[>, == ,<]L}*
    L     ::= T{[+, -]T}* 
    T     ::= D{[*, \]D}*
    D     ::= P{[^] P}* 
    P     ::= '(' E ')' | N | V | FUNC_CALL_AFTER_INIT
    N     ::= ['0' - '9']+
    V     ::= ['a' - 'z', '_']+['a' - 'z', '0' - '9', '_']*
*/

static TreeNode_t* GetG         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack);
static TreeNode_t* GetX         (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack,                                                    SyntaxErr_t* err);
static TreeNode_t* GetFUNC      (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable_func,                        SyntaxErr_t* err);
static TreeNode_t* GetFUNC_DECL (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable_func,                        SyntaxErr_t* err);
static TreeNode_t* GetFUNC_USE  (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable_func,                        SyntaxErr_t* err);
static TreeNode_t* GetBODY      (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable_func,  size_t* nums_of_vars, SyntaxErr_t* err);
static TreeNode_t* GetSTATEMENT (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable,       size_t* nums_of_vars, SyntaxErr_t* err);
static TreeNode_t* GetINIT      (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,       size_t* nums_of_vars, SyntaxErr_t* err);
static TreeNode_t* GetA         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err);
static TreeNode_t* GetIF        (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err);
static TreeNode_t* GetWHILE     (size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err);
static TreeNode_t* GetRETURN    (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err);
static TreeNode_t* GetE         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);
static TreeNode_t* GetL         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);
static TreeNode_t* GetT         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);
static TreeNode_t* GetD         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);
static TreeNode_t* GetP         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);
static TreeNode_t* GetN         (size_t* pos, Tokens_t* tokens,                                                                                           SyntaxErr_t* err);
static TreeNode_t* GetV         (size_t* pos, Tokens_t* tokens,                        Stack_t* stack, name_table* nametable,                             SyntaxErr_t* err, bool is_init);

// -------------------------------------------------------------------------------------

TreeHead_t* MakeLangExprTokens(Tokens_t* tokens, Tokens_t* tokens_copy){
    TreeHead_t* head = TreeCtor();
    Stack_t* stack = stack_ctor(10, __FILE__, __func__, __LINE__); 
    size_t pos = 0;
    head->root = GetG(&pos, tokens, tokens_copy, stack);
    stack_free(stack);
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

static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack){
    SyntaxErr_t err = NO_MISTAKE;
    TreeNode_t* first_op = GetX(pos, tokens, tokens_copy, stack, &err);

    if(err || *pos != tokens->first_free_place){
        if(err) fprintf(stderr, "Syntax error %llu\n", err);
        else    fprintf(stderr, "Position not equal to first free\n", err);
        TokensDtor(tokens);
        TokensDtor(tokens_copy);
        return NULL;
    }
    assert(first_op);
    tree_dump_func(first_op, __FILE__, __func__, __LINE__, tokens->table->nametables[0], "Before ret GetG node %zu", *pos);
    tree_dump_func(first_op, __FILE__, __func__, __LINE__, tokens->table->nametables[1], "Before ret GetG node %zu", *pos);
    return first_op;
}

// -------------------------------------------------------------------------------------
// GetX and helping functions

static TreeNode_t* GetFuncAndNametable(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, SyntaxErr_t* err);

static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, SyntaxErr_t* err){
    assert(err);
    if(*err) return NULL;

    TreeNode_t* node_general = NULL;
    CALL_AND_CHECK_ERR(node_general = GetFuncAndNametable(pos, tokens, tokens_copy, stack, err));

    TreeNode_t* node_left = node_general;

    while(IS_FUNCTION_IN_POS){
        TreeNode_t* node_right = NULL;
        CALL_AND_CHECK_ERR(node_right = GetFuncAndNametable(pos, tokens, tokens_copy, stack, err));

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }
    
    assert(node_general);
    tree_dump_func(node_general, __FILE__, __func__, __LINE__, tokens->table->nametables[0], "Before ret GetX node %zu", *pos);
    return node_general;
}

static TreeNode_t* GetFuncAndNametable(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node = NULL;
    name_table* nametable_func = NameTableInit();
    CALL_AND_CHECK_ERR(node = GetFUNC(pos, tokens, tokens_copy, stack, nametable_func, err));
    TableAddName(tokens->table, nametable_func);

    return node;
}

// -------------------------------------------------------------------------------------

static TreeNode_t* GetFUNC(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable_func, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func_name = NULL;
    CALL_AND_CHECK_ERR(func_name = GetFUNC_DECL(pos, tokens, stack, nametable_func, err));
    size_t num_of_params = nametable_func->first_free - 1;

    TreeNode_t* bodynode = NULL;
    size_t numbers_of_var = 0;
    CALL_AND_CHECK_ERR(bodynode = GetBODY(pos, tokens, tokens_copy, stack, nametable_func, &numbers_of_var, err));

    bodynode -> parent = func_name;
    func_name -> right = bodynode;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(pos, tokens_copy, func_name, OP_CLOSE_FIG_BR, err));

    for(size_t count = 0; count < numbers_of_var + num_of_params; count++){
        CALL_AND_CHECK_ERR(*err = stack_pop(stack, NULL));
    }

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, nametable_func, "Before ret GetFUNC node %zu", *pos);

    return sep_node;
}

// -------------------------------------------------------------------------------------
// GetFuncDecl and GetFuncUse and helping functions

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable_func, bool is_declaration, SyntaxErr_t* err);

static TreeNode_t* GetFUNC_DECL(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable_func,  SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func = NULL;
    CALL_AND_CHECK_ERR(func = GetFUNC_CALL(pos, tokens, stack, nametable_func, true, err));
    return func;
}

static TreeNode_t* GetFUNC_USE(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable_func, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func = NULL;
    CALL_AND_CHECK_ERR(func = GetFUNC_CALL(pos, tokens, stack, nametable_func, false, err));
    return func;
}

// GetFuncCall and helping functions -------------------------------

static void ProcessCloseBracket(TreeNode_t* prev_param, TreeNode_t* current_param);

static void ProcessComma(TreeNode_t* prev_param, TreeNode_t* current_param, TreeNode_t* comma);

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable_func, bool is_declaration, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!(IS_FUNCTION_IN_POS), 
            NO_FUNC_NAME, 
            "No function name\n")

    TreeNode_t* func_name = tokens->node_arr[*pos];
    (*pos)++; // skip FUNC_NAME

    if(!is_declaration){
        func_name->type = FUNC_CALL;
    }

    FAIL_IF(!IS_OPEN_BRACKET_IN_POS,
            NO_OPEN_BR_BEFORE_CALL_FUNC, 
            "No open br before calling function\n")
    (*pos)++; // skip (

    TreeNode_t* prev_param = func_name;

    while(true){
        if(IS_CLOSE_BRACKET_IN_POS){
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* current_param = NULL;
        CALL_AND_CHECK_ERR(current_param = GetE(pos, tokens, stack, nametable_func, err, is_declaration));

        FAIL_IF(!(IS_CLOSE_BRACKET_IN_POS || IS_COMMA_IN_POS),
            NO_CLOSE_BR_OR_COMMA_AFTER_PARAM, 
            "No ( or , after param\n")

        if(IS_CLOSE_BRACKET_IN_POS){
            ProcessCloseBracket(prev_param, current_param);
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* comma = tokens->node_arr[*pos];
        ProcessComma(prev_param, current_param, comma);

        (*pos)++; // skip comma
        prev_param = comma;
    }

    assert(func_name);
    tree_dump_func(func_name, __FILE__, __func__, __LINE__, nametable_func, "Before ret GetFUNCCALL node %zu", *pos);
    return func_name;
}

static void ProcessCloseBracket(TreeNode_t* prev_param, TreeNode_t* current_param){
    if(!prev_param->left){
        prev_param->left = current_param;
    }
    else{
        prev_param->right = current_param;
    }
    current_param -> parent = prev_param;
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

static TreeNode_t* GetBODY(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable_func, size_t* nums_of_vars, SyntaxErr_t* err){
    FAIL_IF(!IS_OPEN_FIGURE_BRACKET_IN_POS, 
            NO_OPEN_FIG_BR_BEFORE_STATEMENT, 
            "No open figure br before statement\n")
    (*pos)++; //skip {

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetSTATEMENT(pos, tokens, tokens_copy, stack, nametable_func, nums_of_vars, err));

    TreeNode_t* node_left = node;
    TreeNode_t* node_right = NULL;

    while(true){
        if(IS_CLOSE_FIGURE_BRACKET_IN_POS){
            (*pos)++; // skip }
            break;
        }

        CALL_AND_CHECK_ERR(node_right = GetSTATEMENT(pos, tokens, tokens_copy, stack, nametable_func, nums_of_vars, err));

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }

    return node;
}

// -------------------------------------------------------------------------------------
// GetSTATEMENT

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable, size_t* nums_of_vars, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node = NULL;
    if(IS_INICIALIZE_IN_POS){
        CALL_AND_CHECK_ERR(node = GetINIT(pos, tokens, stack, nametable, nums_of_vars, err));
    }
    else if(IS_ASSIGN_IN_POS){
        CALL_AND_CHECK_ERR(node = GetA(pos, tokens, stack, nametable, err));
    }
    else if(IS_IF_IN_POS){
        CALL_AND_CHECK_ERR(node = GetIF(pos, tokens, tokens_copy, stack, nametable, err));
    }
    else if(IS_WHILE_IN_POS){
        CALL_AND_CHECK_ERR(node = GetWHILE(pos, tokens,tokens_copy, stack, nametable, err));
    }
    else if(IS_RETURN_IN_POS){
        CALL_AND_CHECK_ERR(node = GetRETURN(pos, tokens, stack, nametable, err));
    }
    else if(IS_FUNCTION_IN_POS){
        CALL_AND_CHECK_ERR(node = GetFUNC_USE(pos, tokens, stack, nametable, err));
    }
    else{
        *err = INCORR_STATEMENT;
        fprintf(stderr, "incorrect statement\n");
        return NULL;
    }
    return node;
}

// -------------------------------------------------------------------------------------
// GetInit or GetA

static TreeNode_t* GetInitOrAssSubtree(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init, size_t* nums_of_vars = NULL);

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, size_t* nums_of_vars, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_INICIALIZE_IN_POS, 
            NO_INIT_OR_ASS_OP, 
            "No init operator in init func\n")

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetInitOrAssSubtree(pos, tokens, stack, nametable, err, true, nums_of_vars));

    return node;
}

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens,Stack_t* stack, name_table* nametable, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_ASSIGN_IN_POS, 
            NO_INIT_OR_ASS_OP, 
            "No assign operator in assign func\n")

    TreeNode_t* node = NULL;
    CALL_AND_CHECK_ERR(node = GetInitOrAssSubtree(pos, tokens, stack, nametable, err, false));

    return node;
}

static TreeNode_t* GetInitOrAssSubtree(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init, size_t* nums_of_vars){
    if(*err) return NULL;

    TreeNode_t* node_left = NULL;
    CALL_AND_CHECK_ERR(node_left = GetV(pos, tokens, stack, nametable, err, is_init));
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INIT or OP_ASS

    TreeNode_t* node_right = NULL;
    CALL_AND_CHECK_ERR(node_right = GetE(pos, tokens, stack, nametable, err, false));

    node->left = node_left;
    node->right = node_right;
    node_left->parent = node;
    node_right->parent = node;

    FAIL_IF(!IS_SEPARATION_POINT_IN_POS, 
            NO_SP_AFTER_INIT_IN_INIT_FUNC, 
            "No ; after inicialization in init func\n")

    TreeNode_t* node_sp = tokens->node_arr[*pos];
    node_sp -> left = node;
    node ->parent = node_sp;

    (*pos)++; // skip ;

    if(is_init) (*nums_of_vars)++; // увеличиваем количество переменных

    return node_sp;
}

// -------------------------------------------------------------------------------------
// GetIf

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable, SyntaxErr_t* err);

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_IF_IN_POS, 
            NO_OP_IF_IN_IF_STAT, 
            "No op if in if statement\n")

    TreeNode_t* if_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip if

    FAIL_IF(!IS_OPEN_BRACKET_IN_POS, 
            NO_OPEN_BR_BEFORE_CONDITION_IN_IF_EXPR, 
            "No open br before condition in if expression\n")
    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_AND_CHECK_ERR(exprnode = GetE(pos, tokens, stack, nametable, err, false));

    if_node -> left = exprnode;
    exprnode -> parent = if_node;

    FAIL_IF(!IS_CLOSE_BRACKET_IN_POS, 
            NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR, 
            "No close br after condition in if expression\n")
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;
    size_t number_if_vars = 0;
    if(IS_OPEN_FIGURE_BRACKET_IN_POS){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, stack, nametable, &number_if_vars, err));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, stack, nametable, &number_if_vars, err));
    }

    if_node -> right = statnode;
    statnode -> parent = if_node;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(pos, tokens_copy, if_node, OP_CLOSE_FIG_BR, err));

    for(size_t count = 0; count < number_if_vars; count++){
        CALL_AND_CHECK_ERR(*err = stack_pop(stack, NULL));
    }

    // else
    TreeNode_t* opnode_else = NULL;
    if(IS_ELSE_IN_POS){
        CALL_AND_CHECK_ERR(opnode_else = GetELSE(pos, tokens, tokens_copy, stack, nametable, err));

        sep_node -> right = opnode_else;
        opnode_else ->parent = sep_node;

        TreeNode_t* sep_node_itog = NULL;
        CALL_AND_CHECK_ERR(sep_node_itog = GetSepNode(pos, tokens_copy, sep_node, OP_CLOSE_FIG_BR, err));

        sep_node = sep_node_itog;
    }
    // ---

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, nametable, "Before ret GetIF node %zu", *pos);
    return sep_node;
}

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_ELSE_IN_POS, 
            NO_OP_ELSE_IN_ELSE_STAT, 
            "No op else in else statement\n")

    TreeNode_t* else_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip else

    TreeNode_t* statnode = NULL;
    size_t num_else_vars = 0;
    if(IS_OPEN_FIGURE_BRACKET_IN_POS){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, stack, nametable, &num_else_vars, err));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, stack, nametable, &num_else_vars, err));
    }

    for(size_t count = 0; count < num_else_vars; count++){
        CALL_AND_CHECK_ERR(*err = stack_pop(stack, NULL));
    }

    else_node -> right = statnode;
    statnode -> parent = else_node;

    tree_dump_func(else_node, __FILE__, __func__, __LINE__, nametable, "Before ret GetELSE node %zu", *pos);

    return else_node;
}

// -------------------------------------------------------------------------------------
// GetWhile

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, Stack_t* stack, name_table* nametable, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_WHILE_IN_POS, 
            NO_OP_WHILE_IN_WHILE_STAT, 
            "No op while in while statement\n")

    TreeNode_t* while_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip while

    FAIL_IF(!IS_OPEN_BRACKET_IN_POS, 
            NO_OPEN_BR_BEFORE_CONDITION_IN_WHILE, 
            "No open br in while expression before condition\n")
    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_AND_CHECK_ERR(exprnode = GetE(pos, tokens, stack, nametable, err, false));

    while_node -> left = exprnode;
    exprnode -> parent = while_node;

    FAIL_IF(!IS_CLOSE_BRACKET_IN_POS, 
            NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE, 
            "No close br in while expression after condition\n")
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;
    size_t num_while_vars = 0;
    if(IS_OPEN_FIGURE_BRACKET_IN_POS){
        CALL_AND_CHECK_ERR(statnode = GetBODY(pos, tokens, tokens_copy, stack, nametable, &num_while_vars, err));
    }
    else{
        CALL_AND_CHECK_ERR(statnode = GetSTATEMENT(pos, tokens, tokens_copy, stack, nametable, &num_while_vars, err));
    }

    for(size_t count = 0; count < num_while_vars; count++){
        CALL_AND_CHECK_ERR(*err = stack_pop(stack, NULL));
    }

    while_node -> right = statnode;
    statnode -> parent = while_node;

    TreeNode_t* sep_node = NULL;
    CALL_AND_CHECK_ERR(sep_node = GetSepNode(pos, tokens_copy, while_node, OP_CLOSE_FIG_BR, err));

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, nametable, "Before ret GetWHILE node %zu", *pos);

    return sep_node;
}

// -------------------------------------------------------------------------------------
// GetReturn

static TreeNode_t* GetRETURN(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err){
    if(*err) return NULL;

    FAIL_IF(!IS_RETURN_IN_POS, 
            NO_RETURN_OP, 
            "No return op in return func\n")

    TreeNode_t* node_ret = tokens->node_arr[*pos];
    (*pos)++; // skip OP_RET

    TreeNode_t* node_e = NULL;
    CALL_AND_CHECK_ERR(node_e = GetE(pos, tokens, stack, nametable, err, false));

    node_ret ->left = node_e;
    node_e -> parent = node_ret;

    FAIL_IF(!IS_SEPARATION_POINT_IN_POS, 
            NO_SP_AFT_RET, 
            "No ; after return\n")
    (*pos)++; // skip OP_SP - не связываем потому что после return в блоке кода ничего идти не должно

    return node_ret;
}

// -------------------------------------------------------------------------------------
// GetE

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetL(pos, tokens, stack, nametable, err, is_init)); 

    while(IS_GE_IN_POS || IS_LE_IN_POS || IS_EQ_IN_POS){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip < or > or ==

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetL(pos, tokens, stack, nametable, err, is_init));

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, nametable, "Before ret GetE node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetL

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetT(pos, tokens, stack, nametable, err, is_init)); 

    while(IS_ADD_IN_POS || IS_SUB_IN_POS){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetT(pos, tokens, stack, nametable, err, is_init));

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, nametable, "Before ret GetE node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetT

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetD(pos, tokens, stack, nametable, err, is_init)); 

    while(IS_MUL_IN_POS || IS_DIV_IN_POS){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip * or /

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetD(pos, tokens, stack, nametable, err, is_init)); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, nametable, "Before ret GetT node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetD

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_AND_CHECK_ERR(left = GetP(pos, tokens, stack, nametable, err, is_init)); 

    while(IS_DEG_IN_POS){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip ^

        TreeNode_t* right = NULL;
        CALL_AND_CHECK_ERR(right = GetP(pos, tokens, stack, nametable, err, is_init)); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, nametable, "Before ret GetD node %zu", *pos);
    return left;
}

// -------------------------------------------------------------------------------------
// GetP

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    if(*err) return NULL;
    TreeNode_t* val = NULL;

    if(IS_OPEN_BRACKET_IN_POS){
        (*pos)++; // skip '('

        CALL_AND_CHECK_ERR(val = GetE(pos, tokens, stack, nametable, err, is_init));

        FAIL_IF(!IS_CLOSE_BRACKET_IN_POS, 
                NO_CLOSE_BRACKET_IN_MATH_EXPR, 
                "No close bracket in math expression\n")
        (*pos)++; // skip ')'
    }
    else{
        if(IS_CONST_IN_POS){
            CALL_AND_CHECK_ERR(val = GetN(pos, tokens, err));
            (*pos)++;
        }
        else if(IS_VARIABLE_IN_POS){
            CALL_AND_CHECK_ERR(val = GetV(pos, tokens, stack, nametable, err, is_init));
            (*pos)++;
        }
        else if(IS_FUNCTION_IN_POS){
            CALL_AND_CHECK_ERR(val = GetFUNC_USE(pos, tokens, stack, nametable, err));
        }
        else{
            *err = INCORR_OPERAND_NOT_VAR_NOT_NUM;
            fprintf(stderr, "Incorrect operand - not num or variable\n");
        } 
    }
    tree_dump_func(val, __FILE__, __func__, __LINE__, nametable, "Before ret GetP node %zu", *pos);
    return val;
}

// -------------------------------------------------------------------------------------
// GetN

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    FAIL_IF(!IS_CONST_IN_POS, 
            NO_CONST, 
            "No const\n")

    return tokens->node_arr[*pos];
}

// -------------------------------------------------------------------------------------
// GetV and hrlping func

static bool FindVarAtStack(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable);

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable, SyntaxErr_t* err, bool is_init){
    FAIL_IF(!IS_VARIABLE_IN_POS, 
            NO_VARIABLE, 
            "No variable\n")

    if(is_init){
        size_t idx = NameTableAddName(nametable, tokens->node_arr[*pos]->var_func_name);
        CALL_AND_CHECK_ERR(*err = stack_push(stack, &idx));
        tokens->node_arr[*pos]->data.var_code = idx;
    }
    else{
        FAIL_IF(!tokens->node_arr[*pos]->var_func_name,
            EMPTY_VAR_NAME,
            "Empty variable name\n");

        bool is_found = FindVarAtStack(pos, tokens, stack, nametable);

        FAIL_IF(!is_found,
            USE_VAR_BEFORE_INIT,
            "Using variable before init\n");
    }

    free(tokens->node_arr[*pos]->var_func_name);
    tokens->node_arr[*pos]->var_func_name = NULL;

    return tokens->node_arr[*pos];
}

static bool FindVarAtStack(size_t* pos, Tokens_t* tokens, Stack_t* stack, name_table* nametable){
    int idx2 = stack->top - 1;
    while(idx2 >= 0 || stack->data[idx2] != stack->front_canary_data){
        if(!strcmp(nametable->var_info[stack->data[idx2]].variable_name, tokens->node_arr[*pos]->var_func_name)){
            tokens->node_arr[*pos]->data.var_code = stack->data[idx2];
            return true;
        }
        idx2--;
    }
    return false;
}

//----------------------------------------------------------------------
// UNDEF DSL

#undef IS_FUNCTION_IN_POS 
#undef IS_VARIABLE_IN_POS 
#undef IS_CONST_IN_POS  
#undef IS_OPEN_BRACKET_IN_POS  
#undef IS_CLOSE_BRACKET_IN_POS 
#undef IS_OPEN_FIGURE_BRACKET_IN_POS
#undef IS_CLOSE_FIGURE_BRACKET_IN_POS
#undef IS_IF_IN_POS  
#undef IS_ELSE_IN_POS 
#undef IS_WHILE_IN_POS 
#undef IS_RETURN_IN_POS  
#undef IS_SEPARATION_POINT_IN_POS  
#undef IS_COMMA_IN_POS  
#undef IS_AND_IN_POS   
#undef IS_OR_IN_POS   
#undef IS_GE_IN_POS    
#undef IS_LE_IN_POS   
#undef IS_EQ_IN_POS   
#undef IS_ADD_IN_POS   
#undef IS_SUB_IN_POS  
#undef IS_MUL_IN_POS     
#undef IS_DIV_IN_POS  
#undef IS_DEG_IN_POS   
#undef IS_INICIALIZE_IN_POS   
#undef IS_ASSIGN_IN_POS    
