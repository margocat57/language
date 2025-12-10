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

//  Еще что поправить - это возврат ошибок
//  Возможно даже сделать отдельный тип для ошибки

// Вопрос - если отличать присваивание от инициализации то как лучше 
// ловить на синтасических ошибках - на токенизации или на синтаксическоом разборе
// пока нигде не ловлю но надо

// универсальная функция(if/else/init/ass) или не надо?

// потенциальная утечка памяти в случае ошибки - обсудить как ее пофиксить
// но связующий узел ; нужен
// пока решила тупо добавять в токены

/*
    Добавить присваивание
    G     ::= X$
    X     ::= {OP}+
    OP    ::= INIT|A|IF|WHILE 

    Пока не разделила - так как мы не можем присвоить в неинициализированную переменную
    INIT  ::= V ':=' E;
    A     ::= V '=' E;

    Может быть сделать универсальное начало для этих всех
    IF    ::= "if" '(' E ')' {X} {ELSE}* 
    ELSE  ::= "else" '(' E ')' {X}
    WHILE ::= "while" '(' E ')' {X}

    Пока обработки операторов > / < / == нет, но будет

    E     ::= T{[+, -]T}* 
    T     ::= D{[*, \]D}*
    D     ::= P{[^] P}* 
    P     ::= '(' E ')' | N | V | F
    N     ::= ['0' - '9']+
    V     ::= ['a' - 'z', '_']+['a' - 'z', '0' - '9', '_']*
*/

static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens);

static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetOP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetA(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);

// static TreeNode_t* GetF(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err);


TreeHead_t* MakeLangExprTokens(Tokens_t* tokens){
    TreeHead_t* head = TreeCtor();
    size_t pos = 0;
    head->root = GetG(&pos, tokens);
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

static TreeNode_t* GetG(size_t* pos, Tokens_t* tokens){
    SyntaxErr_t err = NO_MISTAKE;
    TreeNode_t* first_op = GetX(pos, tokens, &err);

    if(err){
        fprintf(stderr, "Syntax error %llu\n", err);
        TokensDtor(tokens);
        return NULL;
    }
    assert(first_op);
    tree_dump_func(first_op, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetG node %zu", *pos);
    return first_op;
}

static TreeNode_t* GetX(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    assert(err);
    if(*err) return NULL;

    TreeNode_t* node_general = GetOP(pos, tokens, err);
    if(!node_general){
        *err = AT_LEAST_ONE_OP_EXPREXTED;
        fprintf(stderr, "Syntax err - expected at least one operation\n");
        return NULL;
    }

    TreeNode_t* node_left = node_general;

    while((*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_IF || tokens->node_arr[*pos]->data.op == OP_WHILE)) || ((*pos + 1 < tokens->first_free_place && tokens->node_arr[*pos + 1] && tokens->node_arr[*pos + 1]->type == OPERATOR && (tokens->node_arr[*pos + 1]->data.op == OP_ASS || tokens->node_arr[*pos + 1]->data.op == OP_INIT)))){
        TreeNode_t* node_right = GetOP(pos, tokens, err);
        if(!node_right) break;

        node_left -> right   = node_right;
        node_right -> parent = node_left;
        node_left = node_right;
    }
    
    // *pos = local_pos;
    
    assert(node_general);
    tree_dump_func(node_general, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetX node %zu", *pos);
    return node_general;
}


static TreeNode_t* GetOP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;

    TreeNode_t* node = GetINIT(pos, tokens, err);
    if(!node){
        node = GetA(pos, tokens, err);
    }
    if(!node){
        node = GetIF(pos, tokens, err);
        if(node){
            TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_SP}, NULL, node, NULL), tokens);
            TreeNode_t* node_sp = tokens->node_arr[tokens->first_free_place - 1];
            node->parent = node_sp;
            node = node_sp;
        }
    }
    if(!node){
        node = GetWHILE(pos, tokens, err);
        if(node){
            TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OP_SP}, NULL, node, NULL), tokens);
            TreeNode_t* node_sp = tokens->node_arr[tokens->first_free_place - 1];
            node->parent = node_sp;
            node = node_sp;
        }
    }
    if(!node){
        return NULL;
    }
    return node;
}

static TreeNode_t* GetINIT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos + 1]->type == OPERATOR && tokens->node_arr[*pos + 1]->data.op == OP_INIT)){
        return NULL;
    }

    TreeNode_t* node_left = GetV(pos, tokens, err);
    if(!node_left){
        *err = NO_VARIABLE_IN_INIT_FUNC;
        fprintf(stderr, "No variable in init func");
        return NULL;
    }
    (*pos)++; // skip left

    TreeNode_t* node = tokens->node_arr[*pos];
    (*pos)++; // skip OP_INIT

    TreeNode_t* node_right = GetE(pos, tokens, err);
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

    TreeNode_t* node_left = GetV(pos, tokens, err);
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

    TreeNode_t* node_right = GetE(pos, tokens, err);
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

static TreeNode_t* GetIF(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
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

    TreeNode_t* exprnode = GetE(pos, tokens, err);
    assert(if_node);
    if_node -> left = exprnode;
    exprnode -> parent = if_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_IF_EXPR;
        fprintf(stderr, "No close br after condition in if expression %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; // skip )

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
        *err = NO_OPEN_FIGURE_BR_BEFORE_IF;
        fprintf(stderr, "No open figure br before if %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; //skip {

    TreeNode_t* opnode = GetX(pos, tokens, err);
    assert(opnode);
    if_node -> right = opnode;
    opnode -> parent = if_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)){
        *err = NO_CLOSE_FIGURE_BR_AFTER_IF;
        fprintf(stderr, "No close figure br after if");
        return NULL;
    }
    (*pos)++; // skip }

    // else
    TreeNode_t* opnode_else = NULL;
    if(*pos < tokens->first_free_place && tokens->node_arr[*pos] && tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_ELSE){
        opnode_else = GetELSE(pos, tokens, err); 
    }
    // ---

    if(opnode_else){
        TreeNode_t* node_sp = NodeCtor(OPERATOR, (TreeElem_t){.op = OP_SP}, NULL, if_node, opnode_else);
        if_node->parent = node_sp;
        opnode_else -> parent = node_sp;
        if_node = node_sp;
    }
    tree_dump_func(if_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetIF node %zu", *pos);

    return if_node;
}

static TreeNode_t* GetELSE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(*err) return NULL;
    if(*pos + 1 >= tokens->first_free_place) return NULL;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_ELSE)){
        return NULL;
    }

    TreeNode_t* else_node = tokens->node_arr[*pos];
    (*pos) += 1; // skip else

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
        *err = NO_OPEN_FIGURE_BR_BEFORE_ELSE;
        fprintf(stderr, "No open figure br before else expression");
        return NULL;
    }
    (*pos)++; //skip {

    TreeNode_t* opnode = GetX(pos, tokens, err);
    else_node -> right = opnode;
    opnode -> parent = else_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)){
        *err = NO_CLOSE_FIGURE_BR_AFTER_ELSE;
        fprintf(stderr, "No close figure br after else expression");
        return NULL;
    }
    (*pos)++; // skip }
    tree_dump_func(else_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetELSE node %zu", *pos);

    return else_node;
}

static TreeNode_t* GetWHILE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
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

    TreeNode_t* exprnode = GetE(pos, tokens, err);
    while_node -> left = exprnode;
    exprnode -> parent = while_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_BR)){
        *err = NO_CLOSE_BR_AFTER_CONDITION_IN_WHILE;
        fprintf(stderr, "No close br in while expression after condition %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; // skip )

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_FIG_BR)){
        *err = NO_OPEN_FIGURE_BR_BEFORE_WHILE;
        fprintf(stderr, "No open figure br in while expression");
        return NULL;
    }
    (*pos)++; // skip {

    TreeNode_t* opnode = GetX(pos, tokens, err);
    while_node -> right = opnode;
    opnode -> parent = while_node;

    if(!(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_CLOSE_FIG_BR)){
        *err = NO_CLOSE_FIGURE_BR_AFTER_WHILE;
        fprintf(stderr, "No close figure br in while expression %d", tokens->node_arr[*pos]->data.op);
        return NULL;
    }
    (*pos)++; // skip }
    tree_dump_func(while_node, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetWHILE node %zu", *pos);

    return while_node;
}

static TreeNode_t* GetE(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    TreeNode_t* left = GetT(pos, tokens, err); 
    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_ADD || tokens->node_arr[*pos]->data.op == OP_SUB)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip + or -

        TreeNode_t* right = GetT(pos, tokens, err);
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

static TreeNode_t* GetT(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    TreeNode_t* left = GetD(pos, tokens, err);

    while(tokens->node_arr[*pos]->type == OPERATOR && (tokens->node_arr[*pos]->data.op == OP_MUL || tokens->node_arr[*pos]->data.op == OP_DIV)){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip * or /

        TreeNode_t* right = GetD(pos, tokens, err);
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

static TreeNode_t* GetD(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    TreeNode_t* left = GetP(pos, tokens, err);

    while(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_DEG){
        TreeNode_t *new_node = tokens->node_arr[*pos];

        (*pos)++; // skip ^

        TreeNode_t* right = GetP(pos, tokens, err);
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

static TreeNode_t* GetP(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    TreeNode_t* val = NULL;

    if(tokens->node_arr[*pos]->type == OPERATOR && tokens->node_arr[*pos]->data.op == OP_OPEN_BR ){
        (*pos)++; // skip '('

        val = GetE(pos, tokens, err);

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
        if(!val){
            val = GetV(pos, tokens, err);
        }
        if(!val){
            *err = INCORR_OPERAND_NOT_VAR_NOT_NUM;
            fprintf(stderr, "Incorrect operand - not num or variable");
        } 
        (*pos)++;
    }
    assert(val);
    tree_dump_func(val, __FILE__, __func__, __LINE__, tokens->mtk, "Before ret GetP node %zu", *pos);
    return val;
}

static TreeNode_t* GetN(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(tokens->node_arr[*pos]->type != CONST){
        return NULL;
    }
    return tokens->node_arr[*pos];
}

/* static bool FindF(size_t* pos, char* buffer, OPERATORS* op); */

static TreeNode_t* GetV(size_t* pos, Tokens_t* tokens, SyntaxErr_t* err){
    if(tokens->node_arr[*pos]->type != VARIABLE){
        return NULL;
    }
    // чек на функции - потом(а их пока и нет)
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

static TreeNode_t* GetF(size_t* pos, char* buffer, metki *mtk, SyntaxErr_t* err, OPERATORS op){
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