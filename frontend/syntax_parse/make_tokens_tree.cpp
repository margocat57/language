#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "make_tokens_tree.h"

const size_t MAX_BUF_SIZE = 2048;

#define CALL_FUNC_AND_CHECK_ERR_RET_VAL(function, val)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return val;                                                         \
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

/*
    G     ::= X
    X     ::= FUNCTION*
    FUNCTION := FUNC_CALL '{' STATEMENT* '}'
    FUNC_CALL ::= "func_name" (E {, E}*)+ 
    STATEMENT :=  INIT; |  A; | IF | WHILE | RETURN; | '{'STATEMENT*'}' | E; 
    RETURN ::= "return" E;
    IF ::= "if" "(" E ")" STATEMENT ("else" STATEMENT)?
    WHILE  ::= "while" "(" E ")" STATEMENT 
    INIT  ::= V ':=' E;
    A     ::= V '=' E;

    E     ::= L{[>, == ,<]L}*
    L     ::= T{[+, -]T}* 
    T     ::= D{[*, \]D}*
    D     ::= P{[^] P}* 
    P     ::= '(' E ')' | N | V | F
    N     ::= ['0' - '9']+
    V     ::= ['a' - 'z', '_']+['a' - 'z', '0' - '9', '_']*
*/

/*
TODO: сделать область видимости

Как будто стоит помечать параметры что они параметры

В асме нам важно только количество параметров!!!!
мы просто перед вызовом функции пушим их в стрек

Пишем рекурсивный обход дерева в конце if_а если видим узел := то ставим что false

*/

// Двойное копирование - плохо

// параметры функции - мы разрешаем expr или переменную
// и тогда все еще стоит вопрос по области видимости - как мы ее задаем

// А должны ли мы прообегаться с проверкой что у нас нет нулевых узлов сред токенов?

/*
ASM -  как ищем функции?
или мы сначало оходим все дерево а уже потом проставляем функции?

*/

/*
У if/else/while - обойти поддерево и все переменные которые инциализированы сделать невидимыми
Ну область видимости - следующий шаг но важный(без него асм не заведется)

Локальный и глобальный уровень(видна ли переменная сейчас - true/false)- важно для асма
Особенно для функций

Также важно подумать про ретерн и функцию для этого и того как будут связываться стэйтменты

Функции - как проверять допустимое количество параметров? требовать или прототипов

Дальше --- показ кода в пятницу, правки парсинга и начать писать бэкэнд
За чт и пт написать почти рабочий фронтенд
*/


static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy);

static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_params = false);

static TreeNode_t* GetFUNC(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetRETURN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens,  SyntaxErr_t* err);

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init = false, bool is_params = false);

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params);

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params);

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params);

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params);

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init = false, bool is_params = false);


TreeHead_t* MakeLangExprTokens(Tokens_t* tokens, Tokens_t* tokens_copy){
    TreeHead_t* head = TreeCtor();
    size_t pos = 0;
    head->root = GetG(&pos, tokens, tokens_copy);
    if(!head->root){
        return NULL;
    }
    DEBUG_TREE(
    if(TreeVerify(head)){
        fprintf(stderr, "File is not correct - can't work with created tree\n");
        TreeDel(head); 
        return NULL;
    }
    )
    return head;
}

static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy){
    SyntaxErr_t err = NO_MISTAKE;
    TreeNode_t* first_op = GetX(pos, tokens, tokens_copy, &err);

    if(err){
        fprintf(stderr, "Syntax error %llu\n", err);
        TokensDtor(tokens);
        TokensDtor(tokens_copy);
        return NULL;
    }
    assert(first_op);
    tree_dump_func(first_op, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetG node %zu", *pos);
    return first_op;
}


static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy,  SyntaxErr_t* err){
    assert(err);
    if(*err) return NULL;

    TreeNode_t* node_general = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_general = GetFUNC(pos, tokens, tokens_copy, err), NULL);

    if(!node_general){
        *err = AT_LEAST_ONE_OP_EXPREXTED;
        fprintf(stderr, "Syntax err - expected at least one operation\n");
        return NULL;
    }

    TreeNode_t* node_left = node_general;

    while(*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == FUNCTION){
        TreeNode_t* node_right = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_right = GetFUNC(pos, tokens, tokens_copy, err), NULL);

        if(!node_right) break;

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }
    
    assert(node_general);
    tree_dump_func(node_general, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetX node %zu", *pos);
    return node_general;
}

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_params){
    if(*err) return NULL;

    if(!(tokens->node_arr[*pos]->type == FUNCTION)){
        return NULL;
    }

    TreeNode_t* func_name = tokens->node_arr[*pos];
    (*pos)++; // skip FUNC_NAME

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR)){
        *err = NO_OPEN_BR_BEFORE_CALL_FUNC;
        fprintf(stderr, "No open br before calling function\n");
        return NULL;
    }
    (*pos)++; // skip (

    TreeNode_t* prev_param = func_name;

    while(true){
        if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR){
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* current_param = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(current_param = GetE(pos, tokens, err, false, is_params), NULL);

        TreeNode_t* comma = tokens->node_arr[*pos];
        if(comma->type != OPERATOR){
            *err = NO_CLOSE_BR_OR_COMMA_AFTER_PARAM;
            fprintf(stderr, "No ( or , after param\n");
            return NULL;
        }
        if(comma->data.op == OP_CLOSE_BR){
            if(!prev_param->left){
                prev_param->left = current_param;
            }
            else{
                prev_param->right = current_param;
            }
            current_param -> parent = prev_param;
            (*pos)++; // skip )
            break;
        }

        if(!prev_param->left){
            prev_param->left = comma;
        }
        else{
            prev_param->right = comma;
        }
        comma->parent = prev_param;
        comma->left = current_param;
        current_param -> parent = comma;

        (*pos)++; // skip comma

        prev_param = comma;
    }
    return func_name;
}

static TreeNode_t* GetFUNC(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* func_name = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(func_name = GetFUNC_CALL(pos, tokens, err, true), NULL);

    if(!func_name){
        fprintf(stderr, "Can't get func name and params chain\n");
        *err = CANT_GET_FUNC_NAME_AND_PARAMS_CHAIN;
        return NULL;
    }

    /*
        пришлось пилить чтобы не было допустимо 
        int main( ) 
            if(a){
                return a;
            }
    */
    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
        *err = NO_OPEN_BR_BEF_CALL_FUNC;
        fprintf(stderr, "No open br { before calling function\n");
        return NULL;
    }

    TreeNode_t* statnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);
    if(!statnode){
        *err = CANT_GET_NODE_WITH_STATEMENT;
        fprintf(stderr, "Can't get node with statement\n");
        return NULL;
    }

    if(!(tokens->node_arr[*pos - 1]->type == OPERATOR && tokens->node_arr[*pos -1]->data.op == OP_CLOSE_FIG_BR)){
        *err = NO_CLOSE_BR_AFT_CALL_FUNC;
        fprintf(stderr, "No close br } before calling function\n");
        return NULL;
    }

    statnode -> parent = func_name;
    func_name -> right = statnode;

    CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, func_name, NULL), tokens_copy), NULL);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    func_name -> parent = sep_node;

    return sep_node;
}

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetINIT(pos, tokens, err), NULL);
    if(!node){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetA(pos, tokens, err), NULL);
    }
    if(!node){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetIF(pos, tokens, tokens_copy, err), NULL);
    }
    if(!node){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetWHILE(pos, tokens,tokens_copy, err), NULL);
    }
    if(!node){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetRETURN(pos, tokens, err), NULL);
    }
    if(!node){
        if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
            return NULL;
        }
        (*pos)++; //skip {

        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);

        TreeNode_t* node_left = node;
        TreeNode_t* node_right = NULL;

        while(true){
            CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_right = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);
            if(!node_right) break;

            node_left -> right   = node_right;
            node_right -> parent = node_left;
            node_left = node_right;
        }

        if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)){
            *err = NO_CLOSE_FIG_BR_AFTER_STATEMENT;
            fprintf(stderr, "No close figure br after statement\n");
            return NULL;
        }
        (*pos)++; // skip }
    }
    if(!node){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(node = GetE(pos, tokens, err), NULL);
    }
    if(!node){
        return NULL;
    }
    return node;
}

static TreeNode_t* GetRETURN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_RETURN)){
        return NULL;
    }

    TreeNode_t* node_ret = tokens->node_arr[*pos];
    (*pos)++; // skip OP_RET

    TreeNode_t* node_e = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_e = GetE(pos, tokens, err), NULL);

    node_ret ->left = node_e;
    node_e -> parent = node_ret;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = NO_SP_AFT_RET;
        fprintf(stderr, "No ; after return");
        return NULL;
    }
    (*pos)++; // skip OP_SP - не связываем потому что после return в блоке кода ничего идти не должно

    return node_ret;

}

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_INIT)){
        return NULL;
    }

    TreeNode_t* node_left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_left = GetV(pos, tokens, err, true), NULL);
    if(!node_left){
        *err = NO_VARIABLE_IN_INIT_FUNC;
        fprintf(stderr, "No variable in init func\n");
        return NULL;
    }
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INIT

    TreeNode_t* node_right = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_right = GetE(pos, tokens, err), NULL);
    if(!node_right){
        *err = NO_EXPR_IN_INIT_FUNC;
        fprintf(stderr, "No expr in init func\n");
        return NULL;
    }
    // (*pos)++; // skip right

    node->left = node_left;
    node->right = node_right;
    node_left->parent = node;
    node_right->parent = node;
    
    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = NO_SP_AFTER_INIT_IN_INIT_FUNC;
        fprintf(stderr, "No ; after inicialization in init func\n");
        return NULL;
    }

    TreeNode_t* node_sp = tokens->node_arr[*pos];
    node_sp -> left = node;
    node ->parent = node_sp;

    (*pos)++; // skip ;

    return node_sp;

}

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_ASS)){
        return NULL;
    }

    TreeNode_t* node_left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_left = GetV(pos, tokens, err), NULL);
    if(!node_left){
        *err = NO_VARIABLE_IN_ASS_FUNC;
        fprintf(stderr, "No variable in assignment func\n");
        return NULL;
    }
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_ASS

    node->left = node_left;
    node_left->parent = node;

    TreeNode_t* node_right = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(node_right = GetE(pos, tokens, err), NULL);
    if(!node_right){
        *err = NO_EXPR_IN_ASS_FUNC;
        fprintf(stderr, "No expression in assignment func\n");
        return NULL;
    }
    // (*pos)++; // skip right

    node->right = node_right;
    node_right->parent = node;
    
    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = NO_SP_IN_ASS_FUNC;
        fprintf(stderr, "No ; in assignment func %d\n", tokens->node_arr[*pos]->data.op);
        return NULL;
    }

    TreeNode_t* node_sp = tokens->node_arr[*pos];
    node_sp -> left = node;
    node ->parent = node_sp;

    (*pos)++; // skip ;
    tree_dump_func(node_sp, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetA node %zu", *pos);

    return node_sp;
}

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_IF)){
        return NULL;
    }

    TreeNode_t* if_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip if

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR)){
        *err = NO_OPEN_BR_BEFORE_CONDITION_IN_IF_EXPR;
        fprintf(stderr, "No open br before condition in if expression\n");
        return NULL;
    }
    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(exprnode = GetE(pos, tokens, err, false), NULL);

    if_node -> left = exprnode;
    exprnode -> parent = if_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR;
        fprintf(stderr, "No close br after condition in if expression\n");
        return NULL;
    }
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);

    if_node -> right = statnode;
    statnode -> parent = if_node;

    CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, if_node, NULL), tokens_copy), NULL);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    if_node -> parent = sep_node;
    sep_node -> left = if_node;

    // else
    TreeNode_t* opnode_else = NULL;
    if(*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_ELSE){
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(opnode_else = GetELSE(pos, tokens, tokens_copy, err), NULL); 
    }
    // ---

    if(opnode_else){
        sep_node -> right = opnode_else;
        opnode_else ->parent = sep_node;

        CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, sep_node, NULL), tokens_copy), NULL);
        TreeNode_t* sep_node_itog = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
        sep_node -> parent = sep_node_itog;

        sep_node = sep_node_itog;
    }

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetIF node %zu", *pos);
    return sep_node;
}

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_ELSE)){
        return NULL;
    }

    TreeNode_t* else_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip else

    TreeNode_t* statnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);

    else_node -> right = statnode;
    statnode -> parent = else_node;

    tree_dump_func(else_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetELSE node %zu", *pos);

    return else_node;
}

// WHILE  ::= "while" "(" E ")" STATEMENT 
static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_WHILE)){
        return NULL;
    }

    TreeNode_t* while_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip while

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR)){
        *err = NO_OPEN_BR_BEFORE_CONDITION_IN_WHILE;
        fprintf(stderr, "No open br in while expression before condition\n");
        return NULL;
    }

    (*pos)++; // skip (

    TreeNode_t* exprnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(exprnode = GetE(pos, tokens, err, false), NULL);

    while_node -> left = exprnode;
    exprnode -> parent = while_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE;
        fprintf(stderr, "No close br in while expression after condition\n");
        return NULL;
    }
    (*pos)++; // skip )

    TreeNode_t* statnode = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(statnode = GetSTATEMENT(pos, tokens, tokens_copy, err), NULL);

    while_node -> right = statnode;
    statnode -> parent = while_node;

    CHECK_PARSING_ERR(TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, while_node, NULL), tokens_copy), NULL);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    while_node -> parent = sep_node;

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetWHILE node %zu", *pos);

    return sep_node;
}

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(left = GetL(pos, tokens, err, is_init, is_params), NULL); 

    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_GE || tokens->node_arr[*pos]->data.op == OP_LE || tokens->node_arr[*pos]->data.op == OP_EQ)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(right = GetL(pos, tokens, err, is_init, is_params), NULL);

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetE node %zu", *pos);
    return left;
}

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(left = GetT(pos, tokens, err, is_init, is_params), NULL); 

    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_ADD || tokens->node_arr[*pos]->data.op == OP_SUB)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(right = GetT(pos, tokens, err, is_init, is_params), NULL);

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetE node %zu", *pos);
    return left;
}

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(left = GetD(pos, tokens, err, is_init, is_params), NULL); 

    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_MUL || tokens->node_arr[*pos]->data.op == OP_DIV)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip * or /

        TreeNode_t* right = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(right = GetD(pos, tokens, err, is_init, is_params), NULL); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetT node %zu", *pos);
    return left;
}

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(*err) return NULL;

    TreeNode_t* left = NULL;
    CALL_FUNC_AND_CHECK_ERR_RET_VAL(left = GetP(pos, tokens, err, is_init, is_params), NULL); 

    while(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_DEG){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip ^

        TreeNode_t* right = NULL;
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(right = GetP(pos, tokens, err, is_init, is_params), NULL); 

        new_node->left = left;
        new_node->right = right;

        left->parent = new_node;
        right->parent = new_node;

        left = new_node;
    }
    assert(left);
    tree_dump_func(left, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetD node %zu", *pos);
    return left;

}

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(*err) return NULL;
    TreeNode_t* val = NULL;

    if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR ){
        (*pos)++; // skip '('

        CALL_FUNC_AND_CHECK_ERR_RET_VAL(val = GetE(pos, tokens, err, is_init, is_params), NULL);

        if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR ){
            (*pos)++; // skip ')'
        }
        else{
            *err = NO_CLOSE_BRACKET_IN_MATH_EXPR;
            fprintf(stderr, "No close bracket in math expression\n");
            return NULL;
        }
    }
    else{
        CALL_FUNC_AND_CHECK_ERR_RET_VAL(val = GetN(pos, tokens, err), NULL);
        if(val){
            (*pos)++;
        } 
        if(!val){
            CALL_FUNC_AND_CHECK_ERR_RET_VAL(val = GetV(pos, tokens, err, is_init, is_params), NULL);
            if(val){
                (*pos)++;
            } 
        }
        if(!val){
            CALL_FUNC_AND_CHECK_ERR_RET_VAL(val = GetFUNC_CALL(pos, tokens, err), NULL);
        }
        if(!val){
            *err = INCORR_OPERAND_NOT_VAR_NOT_NUM;
            fprintf(stderr, "Incorrect operand - not num or variable\n");
        } 
    }
    tree_dump_func(val, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetP node %zu", *pos);
    return val;
}

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(tokens->node_arr[*pos]->type != CONST){
        return NULL;
    }
    return tokens->node_arr[*pos];
}

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init, bool is_params){
    if(tokens->node_arr[*pos]->type != VARIABLE){
        return NULL;
    }

    size_t idx = 0;

    if(tokens->node_arr[*pos]->var_func_name && !is_params){
        idx = FindVarInNameTable(tokens->mtk, tokens->node_arr[*pos]->var_func_name);
        if(idx == SIZE_MAX && !is_init){
            *err = USE_VAR_BEFORE_INIT;
            fprintf(stderr, "Using variable before inicialization\n");
            return NULL;
        }
    }

    if(is_init || is_params){
        idx = NameTableAddName(tokens->mtk, tokens->node_arr[*pos]->var_func_name);
    }
    if(tokens->node_arr[*pos]->var_func_name){
        free(tokens->node_arr[*pos]->var_func_name);
        tokens->node_arr[*pos]->var_func_name = NULL;
    }

    tokens->node_arr[*pos]->data.var_code = idx;

    return tokens->node_arr[*pos];
}