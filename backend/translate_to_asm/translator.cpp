#include <stdio.h>
#include <assert.h>
#include "translator.h"
#include "../../frontend/include/operators_func.h"

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

#define IS_NODE_PARENT_INIT_OP   (node->parent && node->parent->type == OPERATOR && node->parent->data.op == OP_INIT) 
#define IS_NODE_PARENT_ASSIGN_OP (node->parent && node->parent->type == OPERATOR && node->parent->data.op == OP_ASS) 

#define CREATE_ASM_OP(Op, str)                                                                       \
    static void Create##Op##Asm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){\
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
CREATE_ASM_OP(Deg, "SQRT");

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

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(fp, head->root, mtk, err));
    fclose(fp);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

//-----------------------------------------------------------------------------------------------
static void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, size_t* first_free, int* param_count, TreeErr_t* err);
static void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateConstAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateVariableAsm    (FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateCommaAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk, size_t* first_free, int* param_count, TreeErr_t* err);
static void CreateReturnAsm      (FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateInitAssAsm     (FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);
static void CreateIfAsm          (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* if_count,                        TreeErr_t* err);
static void CreateElseAsm        (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* if_count,                        TreeErr_t* err);
static void CreateWhileAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* if_count,                        TreeErr_t* err);
static void CreateOutputAsm      (FILE *file, TreeNode_t *node, metki_for_translate* mtk,                                       TreeErr_t* err);

//-----------------------------------------------------------------------------------------------
// Recursive function to create assembler

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;
    assert(file); 

    if(!node) return;

    static int if_count = 0;
    static int while_count = 0;
    static int else_count = 0;
    static int param_count = 0;
    static size_t first_free = 0;

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)

    switch(node->type){
        case FUNCTION:      CALL_FUNC_AND_CHECK_ERR(CreateFunctionDeclAsm(file, node, mtk, err));                            break;
        case FUNC_CALL:     CALL_FUNC_AND_CHECK_ERR(CreateFunctionCallAsm(file, node, mtk, &first_free, &param_count, err)); break;
        case FUNCTION_MAIN: CALL_FUNC_AND_CHECK_ERR(CreateFunctionMainAsm(file, node, mtk, err));                            break;
        case CONST:                                 CreateConstAsm(file, node, mtk, err);                                    break;
        case VARIABLE:                              CreateVariableAsm(file, node, mtk, err);                                 break;
        case OPERATOR:
            switch(node->data.op){
                case OP_COMMA:                      CALL_FUNC_AND_CHECK_ERR(CreateCommaAsm(file, node, mtk, &first_free, &param_count, err)); break;
                case OP_RETURN:                     CALL_FUNC_AND_CHECK_ERR(CreateReturnAsm(file, node, mtk, err));                           break;
                case OP_ADD:                        CALL_FUNC_AND_CHECK_ERR(CreateAddAsm(file, node, mtk, err));                              break;
                case OP_SUB:                        CALL_FUNC_AND_CHECK_ERR(CreateSubAsm(file, node, mtk, err));                              break;
                case OP_DIV:                        CALL_FUNC_AND_CHECK_ERR(CreateDivAsm(file, node, mtk, err));                              break;
                case OP_MUL:                        CALL_FUNC_AND_CHECK_ERR(CreateMulAsm(file, node, mtk, err));                              break;
                case OP_DEG:                        CALL_FUNC_AND_CHECK_ERR(CreateDegAsm(file, node, mtk, err));                              break;
                case OP_OUTPUT:                     CALL_FUNC_AND_CHECK_ERR(CreateOutputAsm(file, node, mtk, err));                           break;
                case OP_LE:                         CALL_FUNC_AND_CHECK_ERR(CreateLeAsm(file, node, mtk, err));                               break;
                case OP_GE:                         CALL_FUNC_AND_CHECK_ERR(CreateGeAsm(file, node, mtk, err));                               break; 
                case OP_EQ:                         CALL_FUNC_AND_CHECK_ERR(CreateEqAsm(file, node, mtk, err));                               break; 
                case OP_INIT: case OP_ASS:          CALL_FUNC_AND_CHECK_ERR(CreateInitAssAsm(file, node, mtk, err));                          break; 
                case OP_IF:                         CALL_FUNC_AND_CHECK_ERR(CreateIfAsm(file, node, mtk, &if_count, err));                    break;
                case OP_ELSE:                       CALL_FUNC_AND_CHECK_ERR(CreateElseAsm(file, node, mtk, &else_count, err));                break;
                case OP_WHILE:                      CALL_FUNC_AND_CHECK_ERR(CreateWhileAsm(file, node, mtk, &while_count, err));              break;
                case OP_SP: case OP_CLOSE_FIG_BR:   CALL_FUNC_AND_CHECK_ERR(CreateSpCloseFigBrAsm(file, node, mtk, err));                     break;
                case OP_INPUT:                      fprintf(file, "IN\n");                                                                    break;
                case OP_EXIT:                       fprintf(file, "HLT\n");                                                                   break;
        }
    }

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)
}

//-----------------------------------------------------------------------------------------------
// Realisations of helpful functions

static void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;

    fprintf(file, ":%zu ;%s\n", MetkiAddName(mtk, node->var_func_name), node->var_func_name); 
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

static void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, size_t* first_free, int* param_count, TreeErr_t* err){
    if(*err) return;

    *first_free = node->data.var_code;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "POPR RBX ;сдвигаем на новый стековый фрейм\n"
                    "PUSH %d ; дампим сдвиг в стек чтобы вернуться\n" 
                    "CALL :%zu ;calling %s\n",  node->data.var_code, node->data.var_code, 
                                                MetkiAddName(mtk, node->var_func_name), node->var_func_name); 
    *param_count = 0;
}

static void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;

    mtk->var_info[0].variable_name = node->var_func_name;
    fprintf(file, ":0 ;%s\n", node->var_func_name); 
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

static void CreateConstAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    fprintf(file, "PUSH %d\n", node->data.const_value);
}

static void CreateVariableAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "POPR RCX\n", node->data.var_code); 
    if(node == node->parent->left && (IS_NODE_PARENT_INIT_OP || IS_NODE_PARENT_ASSIGN_OP)) return;
    fprintf(file, "PUSHM [CX]\n");
}

static void CreateCommaAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, size_t* first_free, int* param_count, TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    fprintf(file,   "PUSHR RBX\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "POPR RCX\n"
                    "POPM [CX]\n", *first_free, *param_count); 
    
    (*param_count)++;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

static void CreateReturnAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;

    fprintf(file,   "POPR RDX\n"
                    "PUSHR RBX\n"
                    "PUSHR RDX\n"
                    "SUB\n"
                    "POPR RDX\n"); 

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    fprintf(file,   "PUSHR RDX\n"
                    "POPR RBX\n"    
                    "RET\n");

}

static void CreateIfAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* if_count, TreeErr_t* err){
    if(*err) return;

    char if_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(if_buffer, MAX_SIZE_BUFFER, "if_%d", *if_count);
    (*if_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    fprintf(file, "PUSH 0\n");
    size_t metka_if = MetkiAddName(mtk, if_buffer);
    fprintf(file, "JE :%zu ;%s\n", metka_if, if_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    fprintf(file, ":%zu\n", metka_if);
}

static void CreateElseAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* else_count, TreeErr_t* err){
    if(*err) return;

    char else_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(else_buffer, MAX_SIZE_BUFFER, "else_%d", *else_count);
    (*else_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->parent->left->left, mtk, err)); 

    fprintf(file, "PUSH 0\n");
    size_t metka_else = MetkiAddName(mtk, else_buffer);
    fprintf(file, "JNE :%zu ;%s\n", metka_else, else_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    fprintf(file, ":%zu\n", metka_else);
}

static void CreateWhileAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* while_count, TreeErr_t* err){
    if(*err) return;

    char while_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(while_buffer, MAX_SIZE_BUFFER, "while_cycle_%d", *while_count);
    (*while_count)++;

    size_t metka_while = MetkiAddName(mtk, while_buffer);
    fprintf(file, ":%zu ;%s\n", metka_while, while_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));

    snprintf(while_buffer, MAX_SIZE_BUFFER, "while_jmp_%d", *while_count);
    size_t metka_while1 = MetkiAddName(mtk, while_buffer);

    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE :%zu ;%s\n", metka_while1, while_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));

    fprintf(file, "JMP :%zu\n", metka_while);
    fprintf(file, ":%zu\n", metka_while1);
}


static void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;                                                                      
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));                            
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
}

static void CreateInitAssAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;                                                                      
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, mtk, err));
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file, "POPM [CX]\n");
}

static void CreateOutputAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, mtk, err));
    fprintf(file, "OUT\n");
}

#undef IS_NODE_PARENT_INIT_OP  
#undef IS_NODE_PARENT_ASSIGN_OP 
#undef CREATE_ASM_OP