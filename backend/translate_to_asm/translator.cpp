#include <stdio.h>
#include <assert.h>
#include "translator.h"
#include "../../frontend/include/operators_func.h"

#define CALL_FUNC_AND_CHECK_ERR(function)\
    do{\
        function;\
        if(*err){ \
            fprintf(stderr, "err = %llu, %s, %s, %d\n", *err, __FILE__, __func__, __LINE__); \
            return;                                                         \
        } \
    }while(0)


static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err);

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

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, metki_for_translate* mtk, TreeErr_t* err){
    if(*err) return;
    assert(file); 

    if(!node) return;

    static int if_count = 0;
    static int while_count = 0;
    static int else_count = 0;
    static int param_count = 0;
    static size_t ff = 0;

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)

    switch(node->type){

        case FUNCTION: {     
            size_t metka_func = MetkiAddName(mtk, node->var_func_name);
            fprintf(file, ":%zu ;%s\n", metka_func, node->var_func_name); 
            CreateAsmCodeRecursive(file, node->right, mtk, err);
            break;
        }
        case FUNC_CALL: { 
            ff = node->data.var_code;
            CreateAsmCodeRecursive(file, node->left, mtk, err);
            size_t metka_call = MetkiAddName(mtk, node->var_func_name);
            fprintf(file, "PUSHR RBX\n"); 
            fprintf(file, "PUSH %d\n", node->data.var_code); 
            fprintf(file, "ADD\n");
            fprintf(file, "POPR RBX\n");
            fprintf(file, "PUSH %d\n", node->data.var_code); // дампим сдвиг
            fprintf(file, "CALL :%zu ;calling %s\n", metka_call, node->var_func_name); 
            param_count = 0;
            break;
        }
        case FUNCTION_MAIN: {
            mtk->var_info[0].variable_name = node->var_func_name;
            fprintf(file, ":0 ;%s\n", node->var_func_name); 
            CreateAsmCodeRecursive(file, node->right, mtk, err);
            break;
        }
        case CONST: {       
            fprintf(file, "PUSH %d\n", node->data.const_value); 
            break;
        }
        case VARIABLE: {  
            fprintf(file, "PUSHR RBX\n"); 
            fprintf(file, "PUSH %d\n", node->data.var_code); 
            fprintf(file, "ADD\n");
            fprintf(file, "POPR RCX\n");
            if(node == node->parent->left && node->parent->type == OPERATOR && (node->parent->data.op == OP_INIT || node->parent->data.op == OP_ASS)){
                break;
            }
            else{
                fprintf(file, "PUSHM [CX]\n");
            }
            break;
        }
        case OPERATOR:{
            switch(node->data.op){
                case OP_COMMA: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    fprintf(file, "PUSHR RBX\n"); 
                    fprintf(file, "PUSH %d\n", ff); 
                    fprintf(file, "ADD\n");
                    fprintf(file, "PUSH %d\n", param_count);
                    fprintf(file, "ADD\n");
                    fprintf(file, "POPR RCX\n");
                    fprintf(file, "POPM [CX]\n");
                    param_count++;
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    break;
                }
                case OP_RETURN: {
                    fprintf(file, "POPR RDX\n"); 
                    fprintf(file, "PUSHR RBX\n"); 
                    fprintf(file, "PUSHR RDX\n"); 
                    fprintf(file, "SUB\n");
                    fprintf(file, "POPR RDX\n");
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    fprintf(file, "PUSHR RDX\n");
                    fprintf(file, "POPR RBX\n");    
                    fprintf(file, "RET\n");
                    break;
                }
                case OP_ADD: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "ADD\n");
                    break;
                }
                case OP_SUB: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "SUB\n");
                    break;
                }
                case OP_DIV: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "DIV\n");
                    break;
                }
                case OP_MUL:{
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "MUL\n");
                    break;
                }
                case OP_DEG: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "SQRT\n");
                    break;  
                }
                case OP_EXIT: {
                    fprintf(file, "HLT\n");
                    break;
                }
                case OP_OUTPUT: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    fprintf(file, "OUT\n");
                    break;
                }
                case OP_INPUT: {
                    fprintf(file, "IN\n");
                    break;
                }
                case OP_LE: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "LE\n");
                    break;
                }
                case OP_GE: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "GE\n");
                    break;
                }
                case OP_EQ: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, "EQ\n");
                    break;
                }
                case OP_INIT: case OP_ASS: {
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    fprintf(file, "POPM [CX]\n");
                    break;
                }
                case OP_IF: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    fprintf(file, "PUSH 0\n");
                    char if_buffer[300] = {};
                    snprintf(if_buffer, 299, "if_%d", if_count);
                    if_count++;
                    size_t metka_if = MetkiAddName(mtk, if_buffer);
                    fprintf(file, "JE :%zu ;%s\n", metka_if, if_buffer);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, ":%zu\n", metka_if);
                    break;
                }
                case OP_ELSE: {
                    CreateAsmCodeRecursive(file, node->parent->left->left, mtk, err); // if_condition
                    fprintf(file, "PUSH 0\n");
                    char else_buffer[300] = {};
                    snprintf(else_buffer, 299, "else_%d", else_count);
                    else_count++;
                    size_t metka_else = MetkiAddName(mtk, else_buffer);
                    fprintf(file, "JNE :%zu ;%s\n", metka_else, else_buffer);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    fprintf(file, ":%zu\n", metka_else);
                    break;
                }
                case OP_WHILE: {
                    char while_buffer[300] = {};
                    snprintf(while_buffer, 299, "while_cycle_%d", while_count);
                    size_t metka_while = MetkiAddName(mtk, while_buffer);
                    fprintf(file, ":%zu ;%s\n", metka_while, while_buffer);

                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    char while_buffer1[300] = {};
                    snprintf(while_buffer1, 299, "while_jmp_%d", while_count);
                    while_count++;
                    size_t metka_while1 = MetkiAddName(mtk, while_buffer1);
                    fprintf(file, "PUSH 0\n", metka_while, while_buffer);
                    fprintf(file, "JE :%zu ;%s\n", metka_while1, while_buffer1);

                    CreateAsmCodeRecursive(file, node->right, mtk, err);

                    fprintf(file, "JMP :%zu\n", metka_while);
                    fprintf(file, ":%zu\n", metka_while1);
                    break;
                }
                case OP_SP: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    break;
                }
                case OP_CLOSE_FIG_BR: {
                    CreateAsmCodeRecursive(file, node->left, mtk, err);
                    CreateAsmCodeRecursive(file, node->right, mtk, err);
                    break;
                }
            }

        }

    }

    
    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)
}