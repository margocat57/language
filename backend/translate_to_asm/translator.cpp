#include <stdio.h>
#include <assert.h>
#include "translator.h"
#define TRANSLATOR
#include "../../include/operators_func.h"

const size_t MAX_SIZE_BUFFER = 300;

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err);

//---------------------------------------------------------------------------------
// Define lib

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
            MetkiDestroy(mtk);\
            return;                                                         \
        } \
    }while(0)

#define IS_NODE_PARENT_INIT_OP   (node->parent && node->parent->type == OPERATOR && node->parent->data.op == OP_INIT) 
#define IS_NODE_PARENT_ASSIGN_OP (node->parent && node->parent->type == OPERATOR && node->parent->data.op == OP_ASS) 

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define CREATE_ASM_OP(Op, str)                                                                       \
    void Create##Op##Asm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){\
        if(*err) return;\
        CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));\
        CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));\
        fprintf(file, "%s\n", str);\
    }


CREATE_ASM_OP(Add, "ADD");
CREATE_ASM_OP(Sub, "SUB");
CREATE_ASM_OP(Div, "DIV");
CREATE_ASM_OP(Mul, "MUL");
CREATE_ASM_OP(Eq,  "EQ");
CREATE_ASM_OP(Ge,  "GE");
CREATE_ASM_OP(Le,  "LE");
CREATE_ASM_OP(Deg, "DEG");

#pragma GCC diagnostic pop

//---------------------------------------------------------------------------------------
// Main function for assembly generate

void CreateAsmCode(const char* file_name, const TreeHead_t* head, TreeErr_t* err){
    if(*err) return;
    assert(file_name); assert(head);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)

    FILE* fp = fopen(file_name, "w");
    if(!fp){
        fprintf(stderr, "Can't open output file\n");
        *err = CANT_OPEN_OUT_FILE;
        return;
    }

    metki_for_translate* mtk = MetkiInit();

    fprintf(fp, "CALL :0\n");

    CALL_FUNC_AND_CHECK_ERR_FREE(CreateAsmCodeRecursive(fp, head->root, mtk, err));
    fclose(fp);
    MetkiDestroy(mtk);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

//-----------------------------------------------------------------------------------------------
void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                      TreeErr_t* err);
void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* first_free, int* param_count,   TreeErr_t* err);
void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                      TreeErr_t* err);
void CreateConstAsm       (FILE *file, TreeNode_t *node);
void CreateVariableAsm    (FILE *file, TreeNode_t *node);

//-----------------------------------------------------------------------------------------------
// Recursive function to create assembler

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;
    assert(file); 

    if(!node) return;

    static int if_count = 0;
    static int while_count = 0;
    static int else_count = 0;
    static int param_count = 0;
    static int first_free = 0;
    size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info); 

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)

    switch(node->type){
        case INCORR_VAL:    *err = INCORR_TYPE; break;
        case FUNCTION:      CALL_FUNC_AND_CHECK_ERR(CreateFunctionDeclAsm(file, node, mtk, err)); break;
        case FUNC_CALL:     CALL_FUNC_AND_CHECK_ERR(CreateFunctionCallAsm(file, node, mtk, &first_free, &param_count, err)); break;
        case FUNCTION_MAIN: CALL_FUNC_AND_CHECK_ERR(CreateFunctionMainAsm(file, node, mtk, err)); break;
        case CONST:                                 CreateConstAsm(file, node); break;
        case VARIABLE:                              CreateVariableAsm(file, node); break;
        case OPERATOR:                              if(node->data.op >= num_of_operators){*err = INCORR_OPERATOR;  break;}
            if(node->data.op == OP_COMMA)           {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, mtk, &first_free, &param_count, err));break;} 
            if(node->data.op == OP_IF)              {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, mtk, &if_count, NULL, err)); break;}     
            if(node->data.op == OP_ELSE)            {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, mtk, &else_count, NULL, err)); break;}  
            if(node->data.op == OP_WHILE)           {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, mtk, &while_count, NULL, err)); break;}              
            if(!(OPERATORS_INFO[node->data.op].translate_func)){ *err = INCORR_OPERATOR; break;}
            CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, mtk, NULL, NULL, err)); break;                                                                                                                          break;
        default:                                    *err = INCORR_TYPE; break;
    }

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)
}

//-----------------------------------------------------------------------------------------------
// Realisations of helpful functions

void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;

    fprintf(file, ":%zu ;%s\n", MetkiAddName(mtk, node->var_func_name), node->var_func_name); 

    free(node->var_func_name);
    node->var_func_name = NULL;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* first_free, int* param_count, TreeErr_t* err){
    if(*err) return;

    *first_free = node->data.var_code;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %zu\n"
                    "ADD\n"
                    "POPR RBX ; сдвигаем на новый стековый фрейм\n"
                    "PUSHF %zu ; дампим сдвиг в стек чтобы вернуться\n" 
                    "CALL :%zu ;calling %s\n",  node->data.var_code, node->data.var_code, 
                                                MetkiAddName(mtk, node->var_func_name), node->var_func_name); 
    *param_count = 0;
}

void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;

    mtk->var_info[0].variable_name = strdup(node->var_func_name);
    fprintf(file, ":0 ;%s\n", node->var_func_name); 

    free(node->var_func_name);
    node->var_func_name = NULL;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

void CreateConstAsm(FILE *file, TreeNode_t *node){
    fprintf(file, "PUSH %lg\n", node->data.const_value);
}

void CreateVariableAsm(FILE *file, TreeNode_t *node){
    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %zu\n"
                    "ADD\n"
                    "POPR RCX\n", node->data.var_code); 
    if(node == node->parent->left && (IS_NODE_PARENT_INIT_OP || IS_NODE_PARENT_ASSIGN_OP)) return;
    fprintf(file, "PUSHM [CX]\n");
}

void CreateCommaAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "POPR RCX\n"
                    "POPM [CX]\n", *op_count, *param_count); 
    
    (*param_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

void CreateReturnAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    // сдвигаем регистры
    fprintf(file,   "POPF RDX\n"
                    "PUSHR RBX\n"
                    "PUSHR RDX\n"
                    "SUB\n" 
                    "POPR RDX\n");
    fprintf(file,  "PUSHR RDX\n"
                    "POPR RBX\n"
                    "RET\n");

}

void CreateIfAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;

    char if_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(if_buffer, MAX_SIZE_BUFFER, "if_%d", *op_count);
    (*op_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    fprintf(file, "PUSH 0\n");
    size_t metka_if = MetkiAddName(mtk, if_buffer);
    fprintf(file, "JE :%zu ;%s\n", metka_if, if_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    fprintf(file, ":%zu\n", metka_if);
}

void CreateElseAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;

    char else_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(else_buffer, MAX_SIZE_BUFFER, "else_%d", *op_count);
    (*op_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->parent->left->left, mtk, err)); 

    fprintf(file, "PUSH 0\n");
    size_t metka_else = MetkiAddName(mtk, else_buffer);
    fprintf(file, "JNE :%zu ;%s\n", metka_else, else_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    fprintf(file, ":%zu\n", metka_else);
}

void CreateWhileAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;

    char while_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(while_buffer, MAX_SIZE_BUFFER, "while_cycle_%d", *op_count);
    (*op_count)++;

    size_t metka_while = MetkiAddName(mtk, while_buffer);
    fprintf(file, ":%zu ;%s\n", metka_while, while_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    snprintf(while_buffer, MAX_SIZE_BUFFER, "while_jmp_%d", *op_count);
    size_t metka_while1 = MetkiAddName(mtk, while_buffer);

    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE :%zu ;%s\n", metka_while1, while_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));

    fprintf(file, "JMP :%zu\n", metka_while);
    fprintf(file, ":%zu\n", metka_while1);
}


void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;                                                                    
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));                            
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

void CreateInitAssAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;                                                                      
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file, "POPM [CX]\n");
}

void CreateOutputAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    if(*err) return;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file, "OUT\n");
}

void CreateInputAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    fprintf(file, "IN\n"); 
}

void CreateExitAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count, TreeErr_t* err){
    fprintf(file, "HLT\n RET\n");
}

#pragma GCC diagnostic pop

#undef IS_NODE_PARENT_INIT_OP  
#undef IS_NODE_PARENT_ASSIGN_OP 
#undef CREATE_ASM_OP
#undef TRANSLATOR