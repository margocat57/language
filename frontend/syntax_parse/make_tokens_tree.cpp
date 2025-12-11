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

/*
    G     ::= X
    X     ::= FUNCTION*
    FUNCTION := FUNC_CALL STATEMENT*
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

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetFUNC(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetRETURN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens,  SyntaxErr_t* err);

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err);

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init);


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

    TreeNode_t* node_general = GetFUNC(pos, tokens, tokens_copy, err);
    if(!node_general){
        *err = AT_LEAST_ONE_OP_EXPREXTED;
        fprintf(stderr, "Syntax err - expected at least one operation\n");
        return NULL;
    }

    TreeNode_t* node_left = node_general;

    fprintf(stderr, "*pos = %zu, elem = %d", *pos, tokens->node_arr[*pos]->type );

    while(*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == FUNCTION){
        TreeNode_t* node_right = GetFUNC(pos, tokens, tokens_copy, err);

        if(*err){
            return NULL;
        } 
        if(!node_right){
            break;
        } 

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }
    
    assert(node_general);
    tree_dump_func(node_general, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetX node %zu", *pos);
    return node_general;
}

static TreeNode_t* GetFUNC_CALL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(!(tokens->node_arr[*pos]->type == FUNCTION)){
        fprintf(stderr, "\nmeiwa %zu\n", *pos);
        return NULL;
    }

    TreeNode_t* func_name = tokens->node_arr[*pos];
    (*pos)++; // skip FUNC_NAME

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR)){
        *err = INCORR_FILE;
        fprintf(stderr, "No open br before calling function");
        return NULL;
    }
    (*pos)++; // skip (

    TreeNode_t* prev_param = func_name;

    while(true){
        if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR){
            (*pos)++; // skip )
            break;
        }

        TreeNode_t* current_param = GetE(pos, tokens, err, false);

        TreeNode_t* comma = tokens->node_arr[*pos];
        if(comma->type != OPERATOR){
            *err = INCORR_FILE;
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
    TreeNode_t* func_name = GetFUNC_CALL(pos, tokens, err);

    if(!func_name){
        *err = INCORR_FILE;
        return NULL;
    }

    TreeNode_t* statnode = GetSTATEMENT(pos, tokens, tokens_copy, err);
    statnode -> parent = func_name;
    func_name -> right = statnode;

    TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, func_name, NULL), tokens_copy);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    func_name -> parent = sep_node;

    return sep_node;
}

static TreeNode_t* GetSTATEMENT(size_t* pos, Tokens_t* tokens, Tokens_t* tokens_copy, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node = GetINIT(pos, tokens, err);
    if(!node){
        node = GetA(pos, tokens, err);
    }
    if(!node){
        node = GetIF(pos, tokens, tokens_copy, err);
    }
    if(!node){
        node = GetWHILE(pos, tokens,tokens_copy, err);
    }
    if(!node){
        node = GetRETURN(pos, tokens, err);
    }
    if(!node){
        if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
            return NULL;
        }
        (*pos)++; //skip {

        node = GetSTATEMENT(pos, tokens, tokens_copy, err);

        TreeNode_t* node_left = node;
        TreeNode_t* node_right = NULL;

        while(true){
            node_right = GetSTATEMENT(pos, tokens, tokens_copy, err);
            if(!node_right) break;

            node_left -> right   = node_right;
            node_right -> parent = node_left;
            node_left = node_right;
        }

        if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)){
            *err = INCORR_FILE;
            fprintf(stderr, "No close figure br after statement");
            return NULL;
        }
        fprintf(stderr, "\n%zu - close br pos\n", *pos);
        (*pos)++; // skip }
    }
    if(!node){
        node = GetE(pos, tokens, err, false);
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

    TreeNode_t* node_e = GetE(pos, tokens, err, false);

    node_ret ->left = node_e;
    node_e -> parent = node_ret;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = INCORR_FILE;
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

    TreeNode_t* node_left = GetV(pos, tokens, err, true);
    if(!node_left){
        *err = NO_VARIABLE_IN_INIT_FUNC;
        fprintf(stderr, "No variable in init func");
        return NULL;
    }
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INIT

    TreeNode_t* node_right = GetE(pos, tokens, err, true);
    if(!node_right){
        *err = NO_EXPR_IN_INIT_FUNC;
        fprintf(stderr, "No expr in init func");
        return NULL;
    }
    // (*pos)++; // skip right

    node->left = node_left;
    node->right = node_right;
    node_left->parent = node;
    node_right->parent = node;
    
    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = NO_SP_AFTER_INIT_IN_INIT_FUNC;
        fprintf(stderr, "No ; after inicialization in init func");
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

    TreeNode_t* node_left = GetV(pos, tokens, err, false);
    if(!node_left){
        *err = NO_VARIABLE_IN_ASS_FUNC;
        fprintf(stderr, "No variable in assignment func");
        return NULL;
    }
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_ASS
    node->left = node_left;
    node_left->parent = node;

    TreeNode_t* node_right = GetE(pos, tokens, err, false);
    if(!node_right){
        *err = NO_EXPR_IN_ASS_FUNC;
        fprintf(stderr, "No expression in assignment func");
        return NULL;
    }
    // (*pos)++; // skip right

    node->right = node_right;
    node_right->parent = node;
    
    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_SP)){
        *err = NO_SP_IN_ASS_FUNC;
        fprintf(stderr, "No ; in assignment func %d", tokens->node_arr[*pos]->data.op);
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
        fprintf(stderr, "No open br before condition in if expression");
        return NULL;
    }
    (*pos)++; // skip (

    TreeNode_t* exprnode = GetE(pos, tokens, err, false);
    assert(if_node);
    if_node -> left = exprnode;
    exprnode -> parent = if_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR;
        fprintf(stderr, "No close br after condition in if expression %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; // skip )

    TreeNode_t* statnode = GetSTATEMENT(pos, tokens, tokens_copy, err);
    if_node -> right = statnode;
    statnode -> parent = if_node;

    TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, if_node, NULL), tokens_copy);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    if_node -> parent = sep_node;

    // else
    TreeNode_t* opnode_else = NULL;
    if(*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_ELSE){
        opnode_else = GetELSE(pos, tokens, tokens_copy, err); 
    }
    // ---

    if(opnode_else){
        sep_node -> right = opnode_else;
        opnode_else ->parent = sep_node;

        TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, if_node, NULL), tokens_copy);
        TreeNode_t* sep_node_itog = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
        if_node -> parent = sep_node_itog;

        sep_node = sep_node_itog;
    }
    tree_dump_func(if_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetIF node %zu", *pos);

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

    TreeNode_t* statnode = GetSTATEMENT(pos, tokens, tokens_copy, err);
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
        fprintf(stderr, "No open br in while expression before condition");
        return NULL;
    }

    (*pos)++; // skip (

    TreeNode_t* exprnode = GetE(pos, tokens, err, false);
    while_node -> left = exprnode;
    exprnode -> parent = while_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE;
        fprintf(stderr, "No close br in while expression after condition %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; // skip )

    fprintf(stderr, "pos while = %zu", *pos);
    TreeNode_t* statnode = GetSTATEMENT(pos, tokens, tokens_copy, err);
    while_node -> right = statnode;
    statnode -> parent = while_node;

    TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_CLOSE_FIG_BR}, NULL, while_node, NULL), tokens_copy);
    TreeNode_t* sep_node = tokens_copy->node_arr[tokens_copy->first_free_place - 1];
    while_node -> parent = sep_node;

    tree_dump_func(sep_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetWHILE node %zu", *pos);

    return sep_node;
}

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    TreeNode_t* left = GetL(pos, tokens, err, is_init); 
    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_GE || tokens->node_arr[*pos]->data.op == OP_LE || tokens->node_arr[*pos]->data.op == OP_EQ)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = GetL(pos, tokens, err, is_init);
        if(*err){
            return NULL;
        }

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

static TreeNode_t* GetL(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    TreeNode_t* left = GetT(pos, tokens, err, is_init); 
    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_ADD || tokens->node_arr[*pos]->data.op == OP_SUB)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = GetT(pos, tokens, err, is_init);
        if(*err){
            return NULL;
        }

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

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    TreeNode_t* left = GetD(pos, tokens, err, is_init);

    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_MUL || tokens->node_arr[*pos]->data.op == OP_DIV)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip * or /

        TreeNode_t* right = GetD(pos, tokens, err, is_init);
        if(*err){
            return NULL;
        }

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

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    TreeNode_t* left = GetP(pos, tokens, err, is_init);

    while(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_DEG){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip ^

        TreeNode_t* right = GetP(pos, tokens, err, is_init);
        if(*err){
            return NULL;
        }

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

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    TreeNode_t* val = NULL;

    if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR ){
        (*pos)++; // skip '('

        val = GetE(pos, tokens, err, is_init);

        if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR ){
            (*pos)++; // skip ')'
        }
        else{
            *err = NO_CLOSE_BRACKET_IN_MATH_EXPR;
            fprintf(stderr, "No close bracket in math expression");
            return NULL;
        }
    }
    else{
        val = GetN(pos, tokens, err);
        if(val){
            (*pos)++;
        } 
        if(!val){
            val = GetV(pos, tokens, err, is_init);
            if(val){
                (*pos)++;
            } 
        }
        if(!val){
            val = GetFUNC_CALL(pos, tokens, err);
            fprintf(stderr, "posf = %zu %p", *pos, val);
        }
        if(!val){
            *err = INCORR_OPERAND_NOT_VAR_NOT_NUM;
            fprintf(stderr, "Incorrect operand - not num or variable");
        } 
    }
    tree_dump_func(val, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetP node %zu", *pos);
    return val;
}

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    fprintf(stderr, "posN = %zu", *pos);
    if(tokens->node_arr[*pos]->type != CONST){
        return NULL;
    }
    return tokens->node_arr[*pos];
}

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err, bool is_init){
    fprintf(stderr, "posV = %zu", *pos);
    if(tokens->node_arr[*pos]->type != VARIABLE){
        return NULL;
    }

    size_t idx = 0;

    if(tokens->node_arr[*pos]->data.var_func_name){
        idx = FindVarInNameTable(tokens->mtk, tokens->node_arr[*pos]->data.var_func_name);
        if(idx == SIZE_MAX && !is_init){
            *err = USE_VAR_BEFORE_INIT;
            fprintf(stderr, "Using variable before inicialization");
            return NULL;
        }
    }

    if(is_init){
        idx = NameTableAddName(tokens->mtk, tokens->node_arr[*pos]->data.var_func_name);
    }
    tokens->node_arr[*pos]->data.var_func_name = NULL;
    tokens->node_arr[*pos]->data.var_code = idx;

    return tokens->node_arr[*pos];
}

/*
static bool FindF(size_t* pos, char* buffer, OPERATORS* op){
    size_t num_of_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
    for(size_t idx = 1; idx < num_of_op; idx++){
        if(OPERATORS_INFO[idx].op == OP_ADD || OPERATORS_INFO[idx].op == OP_SUB || OPERATORS_INFO[idx].op == OP_MUL || OPERATORS_INFO[idx].op == OP_DIV || OPERATORS_INFO[idx].op == OP_DEG){
            continue;
        }
        if(!strncmp(buffer + *pos, OPERATORS_INFO[idx].op_name, OPERATORS_INFO[idx].num_of_symb)){
            (*pos) += OPERATORS_INFO[idx].num_of_symb;
            *op = OPERATORS_INFO[idx].op;
            return true;
        }
    }
    return false;
}

static TreeNode_t* GetF(size_t* pos, char* buffer, name_table *mtk, SyntaxErr_t* err, OPERATORS op){
    TreeNode_t* val = NULL;
    skip_space(buffer, pos); 

    if(buffer[*pos] == '('){
        (*pos)++;
        TreeNode_t* left = GetE(pos, buffer, mtk, err);
        if(*err){
            TreeDelNodeRecur(left);
            return NULL;
        }
        skip_space(buffer, pos); 
        if(buffer[*pos] != ')'){
            *err = INCORR_FILE;
            TreeDelNodeRecur(left);
            return NULL;
        }
        (*pos)++;

        val = NodeCtor(OPERATOR, (TreeElem_t){.op = op}, NULL, left, NULL);
        left->parent = val;
    }
    else{
        *err = INCORR_FILE;
    }
    return val;
}
*/