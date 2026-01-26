#include <stdio.h>
#include <assert.h>
#include "translator.h"
#define TRANSLATOR
#include "../../include/operators_func.h"

/*
Короче здесь может быть стоит
сделать структуру с счетчиками
и добавлять и убирать конкретный счетчик
ну типо когда +1 иф то увеличиваем иф каунт и тд
но изза наличия вложенных ифов и циклов стоит копировать
*/

const size_t MAX_SIZE_BUFFER = 300;

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err);

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define CREATE_ASM_OP(Op, str)                                                                       \
    void Create##Op##Asm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){\
        if(*err) return;\
        CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));\
        CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));\
        fprintf(file, "%s\n", str);\
    }


CREATE_ASM_OP(Add, "ADD");
CREATE_ASM_OP(Sub, "SUB");
CREATE_ASM_OP(Div, "DIV");
CREATE_ASM_OP(Mul, "MUL");
CREATE_ASM_OP(Eq,  "EQ");
CREATE_ASM_OP(Ge,  "GE");
CREATE_ASM_OP(Le,  "LE");
CREATE_ASM_OP(Geq, "GEQ");
CREATE_ASM_OP(Leq, "LEQ");
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


    fprintf(fp, "CALL :Che_grande_cazzo_main\n");
    op_counters counters = {};

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(fp, head->root, &counters, err));
    fclose(fp);

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeVerify(head));)
}

//-----------------------------------------------------------------------------------------------
void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err);
void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err);
void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, op_counters* counters,TreeErr_t* err);
void CreateConstAsm       (FILE *file, TreeNode_t *node, op_counters* counters);
void CreateVariableAsm    (FILE *file, TreeNode_t *node, op_counters* counters);

//-----------------------------------------------------------------------------------------------
// Recursive function to create assembler

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void CreateAsmCodeRecursive(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;
    assert(file); 

    if(!node) return;

    size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info); 

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)

    switch(node->type){
        case INCORR_VAL:    *err = INCORR_TYPE; break;
        case FUNCTION:      CALL_FUNC_AND_CHECK_ERR(CreateFunctionDeclAsm(file, node, counters, err)); break;
        case FUNC_CALL:     CALL_FUNC_AND_CHECK_ERR(CreateFunctionCallAsm(file, node, counters, err)); break;
        case FUNCTION_MAIN: CALL_FUNC_AND_CHECK_ERR(CreateFunctionMainAsm(file, node, counters, err)); break;
        case CONST:                                 CreateConstAsm(file, node, counters); break;
        case VARIABLE:                              CreateVariableAsm(file, node, counters); break;
        case OPERATOR:                              if(node->data.op >= num_of_operators){*err = INCORR_OPERATOR;  break;}
            if(node->data.op == OP_COMMA)           {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, counters, err));break;} 
            if(node->data.op == OP_IF)              {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, counters, err)); break;}     
            if(node->data.op == OP_ELSE)            {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, counters, err)); break;}  
            if(node->data.op == OP_WHILE)           {CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, counters, err)); break;}              
            if(!(OPERATORS_INFO[node->data.op].translate_func)){ *err = INCORR_OPERATOR;  break;}
            CALL_FUNC_AND_CHECK_ERR(OPERATORS_INFO[node->data.op].translate_func(file, node, counters,  err)); break;                                                                                                                          break;
        default:                                    *err = INCORR_TYPE; break;
    }

    DEBUG_TREE(CALL_FUNC_AND_CHECK_ERR(*err = TreeNodeVerify(node));)
}

//-----------------------------------------------------------------------------------------------
// Realisations of helpful functions

void CreateFunctionDeclAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    fprintf(file, ":%s\n", node->var_func_name); 

    free(node->var_func_name);
    node->var_func_name = NULL;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
}

void CreateFunctionCallAsm(FILE *file, TreeNode_t *node, op_counters* counters,  TreeErr_t* err){
    if(*err) return;

    counters->first_free = node->data.var_code; //вот это я вообще не поняла
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));
    fprintf(file,   "PUSHR RBX ;start shift RBX\n"
                    "PUSH %zu\n"
                    "ADD\n"
                    "POPR RBX\n"
                    "PUSHF %zu ;end shift RBX\n" 
                    "CALL :%s\n",  node->data.var_code, node->data.var_code, node->var_func_name); 
    counters->param_count = 0;

}

void CreateFunctionMainAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    fprintf(file, ":Che_grande_cazzo_main\n"); 

    free(node->var_func_name);
    node->var_func_name = NULL;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));

}

void CreateConstAsm(FILE *file, TreeNode_t *node, op_counters* counters){
    fprintf(file, "PUSH %lg\n", node->data.const_value);
}

void CreateVariableAsm(FILE *file, TreeNode_t *node, op_counters* counters){
    fprintf(file,   "PUSHR RBX ;start woking with variable\n"
                    "PUSH %zu\n"
                    "ADD\n"
                    "POPR RCX\n", node->data.var_code); 
    if(node == node->parent->left && (IS_NODE_PARENT_INIT_OP || IS_NODE_PARENT_ASSIGN_OP)) return;
    fprintf(file, "PUSHM [CX]\n");
}

void CreateCommaAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));

    fprintf(file,   "PUSHR RBX ;copying variable start\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "PUSH %d\n"
                    "ADD\n"
                    "POPR RCX\n"
                    "POPM [CX] ;copying variable end\n", counters->first_free, counters->param_count); 
    
    (counters->param_count)++;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
}

void CreateReturnAsm(FILE *file, TreeNode_t *node, op_counters* counters,  TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));

    // сдвигаем регистры
    fprintf(file,   "POPF RDX  ;start copying shift to return\n"
                    "PUSHR RBX\n"
                    "PUSHR RDX\n"
                    "SUB\n" 
                    "POPR RDX\n"
                    "PUSHR RDX\n"
                    "POPR RBX ;end copying shift to return\n"
                    "RET\n");

}

void CreateIfAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    bool has_else = false;

    char if_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(if_buffer, MAX_SIZE_BUFFER, "if_%d", counters->if_count);
    (counters->if_count)++;

    char else_buffer[MAX_SIZE_BUFFER] = {};

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));

    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE :%s\n", if_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
    if(node->parent && node->parent->right && node->parent->right->type == OPERATOR && node->parent->right->data.op == OP_ELSE){
        has_else = true;

        snprintf(else_buffer, MAX_SIZE_BUFFER, "else_%d", counters->else_count);
        (counters->else_count)++;

        fprintf(file, "JMP :%s\n", else_buffer);
    }
    fprintf(file, ":%s ;end if\n", if_buffer);
    if(has_else) CALL_FUNC_AND_CHECK_ERR(CreateElseAsm(file, node->parent->right, counters, err));
    if(has_else){
        fprintf(file, ":%s ;end else \n", else_buffer);
    } 

}

void CreateElseAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
}

void CreateWhileAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;

    char while_buffer[MAX_SIZE_BUFFER] = {};
    snprintf(while_buffer, MAX_SIZE_BUFFER, "while_cycle_%d", counters->while_count);
    (counters->while_count)++;

    fprintf(file, ":%s\n", while_buffer);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));

    char while_buffer1[MAX_SIZE_BUFFER] = {};
    snprintf(while_buffer1, MAX_SIZE_BUFFER, "while_jmp_%d", counters->while_count);

    fprintf(file, "PUSH 0\n");
    fprintf(file, "JE :%s\n", while_buffer1);

    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));

    fprintf(file, "JMP :%s ; end while\n", while_buffer);
    fprintf(file, ":%s\n", while_buffer1);
}


void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;                                                                    
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));

    if(node->right && node->right->type == OPERATOR && node->right->data.op == OP_ELSE)  return;                         
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
}

void CreateInitAssAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;                                                                   
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));
    fprintf(file, "POPM [CX]\n");
}

void CreateOutputAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    if(*err) return;
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->left, counters, err));
    fprintf(file, "OUT\n");
}

void CreateRamDumpAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    fprintf(file, "DRAW\n");
    CALL_FUNC_AND_CHECK_ERR(CreateAsmCodeRecursive(file, node->right, counters, err));
}

void CreateInputAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    fprintf(file, "IN\n"); 
}

void CreateExitAsm(FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err){
    fprintf(file, "HLT\nRET\n");
}

#pragma GCC diagnostic pop

#undef IS_NODE_PARENT_INIT_OP  
#undef IS_NODE_PARENT_ASSIGN_OP 
#undef CREATE_ASM_OP
#undef TRANSLATOR