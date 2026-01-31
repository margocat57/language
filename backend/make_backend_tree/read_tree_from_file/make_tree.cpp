#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h> 
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../tree/tree_func.h"
#include "../../../tree/mistakes.h"
#include "../../../debug_output/graphviz_dump.h"
#include "../../../include/operators_func.h"
#include "../../../include/standart_func.h"

const size_t BUFFER_LEN = 300;

//----------------------------------------------------------------------------
// Helping functions 

// To find spaces -----------------------------------
static void skip_space(char* str, size_t* pos){
    assert(str);
    assert(pos);

    char ch = str[(*pos)];
    while(isspace(ch) && ch != '\0'){
        (*pos)++;
        ch = str[(*pos)];
    }
}


// Connect with parents -----------------------------------
static void ConnectWithParents(TreeNode_t *node){
    if(!node) return;

    ConnectWithParents(node->left);
    ConnectWithParents(node->right);

    if(node->left){
        node->left->parent = node;
    }
    if(node->right){
        node->right->parent = node;
    }
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Creating buffer for reading tree info from disk

static bool is_stat_err(const char *name_of_file, struct stat *all_info_about_file);

static char* read_file_to_string_array(const char *name_of_file){
    assert(name_of_file != NULL);
    FILE *fptr = fopen(name_of_file, "r");
    if(!fptr){
        fprintf(stderr, "Can't open file '%s' for reading. Error: %s\n", 
            name_of_file, strerror(errno));
        return NULL;
    }

    struct stat file_info = {};
    if(is_stat_err(name_of_file, &(file_info))){
        return NULL;
    }

    char *all_strings_in_file = (char *)calloc(file_info.st_size + 1, sizeof(char));
    if(!all_strings_in_file){
        fprintf(stderr, "Array for strings allocation error\n");
        return NULL;
    }

    if(fread(all_strings_in_file, sizeof(char), file_info.st_size, fptr) != file_info.st_size){
        fprintf(stderr, "Can't read all symbols from file\n");
        return NULL;
    }

    fclose(fptr);
    return all_strings_in_file;
}

static bool is_stat_err(const char *name_of_file, struct stat *all_info_about_file){
    assert(name_of_file != NULL);
    assert(all_info_about_file != NULL);

    if (stat(name_of_file, all_info_about_file) == -1){
        perror("Stat error");
        fprintf(stderr, "Error code: %d\n", errno);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Make tree

static void ReadNode(size_t* pos, char* buffer, TreeNode_t** node_to_write, TreeErr_t* err);

TreeHead_t* MakeBackendTree(const char *name_of_file){
    assert(name_of_file);

    char* buffer = read_file_to_string_array(name_of_file);
    if(!buffer){
        return NULL;
    }
    TreeHead_t* head = TreeCtor();
    size_t pos = 0;
    TreeErr_t err = NO_MISTAKE;
    ReadNode(&pos, buffer, &(head->root), &err);
    if(err){
        TreeDel(head);
        free(buffer);
        return NULL;
    }
    DEBUG_TREE(
    if(TreeVerify(head)){
        fprintf(stderr, "File is not correct - can't work with created tree\n");
        TreeDel(head);
        free(buffer);
        return NULL;
    }
    )
    free(buffer);
    ConnectWithParents(head->root);
    tree_dump_func(head->root, __FILE__, __func__, __LINE__, "Debug backend tree\n");
    return head;
}

static TreeNode_t* ReadHeader(size_t* pos, char* buffer, TreeErr_t* err);

static void ReadNode(size_t* pos, char* buffer, TreeNode_t** node_to_write, TreeErr_t* err){
    assert(pos);
    assert(err);
    assert(buffer);
    assert(node_to_write);

    if(*err) return;

    skip_space(buffer, pos);
    if(buffer[(*pos)] == '('){
        (*pos)++; //skip '('
        skip_space(buffer, pos);

        if(buffer[(*pos)] != '"'){
            fprintf(stderr, "Incorr file no \"\n");
            *err = NO_QUOTES_BEFORE_NEW_NODE;
            return;
        }

        *node_to_write = ReadHeader(pos, buffer, err); 
        skip_space(buffer, pos);
        if(!*node_to_write || *err){
            *err = UNDEFINED_NODE_TYPE;
            return;
        }

        ReadNode(pos, buffer, &((*node_to_write)->left), err);
        if(*err){
            fprintf(stderr, "Incorr file while reading left node\n");
            return;
        }

        ReadNode(pos, buffer, &((*node_to_write)->right), err);
        if(*err){
            fprintf(stderr, "Incorr file  while reading right node\n");
            return;
        }

        skip_space(buffer, pos);
        if(buffer[(*pos)] != ')'){
            fprintf(stderr, "Incorr file no )\n");
            *err = NO_CLOSE_BR_IN_FILE;
            return;
        }
        (*pos)++; //skip ')'
        skip_space(buffer, pos);
        return;
    }
    else if(!strncmp(buffer + *pos, "nil", 3)){
        (*pos) += strlen("nil");
        *node_to_write = NULL;
        skip_space(buffer, pos);
        return;
    }
    fprintf(stderr, "Incorr file undef symbol(%s), pos(%zu)\n", buffer + *pos, *pos);
    *err = UNDEFINED_SYMBOL;
    return;
}

//-------------------------------------------------------------------------------------------
// Reading string from file

// на будущее - дампить не по типу а число в енаме и делать свитч по числу

#define CREATE_PARSER_FUNCTION(func_name, prefix, node_type) \
    static TreeNode_t* parse_##func_name(const char* buffer) { \
        if(!strncmp(prefix, buffer, strlen(prefix))) { \
            size_t line = 0;\
            ssize_t position_in_line = 0;\
            char name[BUFFER_LEN] = {}; \
            sscanf(buffer, prefix " %s line %zu pos %zd", name, &line, &position_in_line); \
            return NodeCtor(node_type, {}, NULL, NULL, NULL, strdup(name), line, position_in_line); \
        }\
        return NULL;\
    }

CREATE_PARSER_FUNCTION(variable, "VAR", VARIABLE)
CREATE_PARSER_FUNCTION(func_call, "CALL", FUNC_CALL)
CREATE_PARSER_FUNCTION(function, "FUNC", FUNCTION)
CREATE_PARSER_FUNCTION(function_main, "MAIN", FUNCTION_MAIN)

static TreeNode_t* parse_digit(const char* buffer);

static TreeNode_t* parse_op(const char* buffer, TreeErr_t* err);

static TreeNode_t* parse_std_func(const char* buffer, TreeErr_t* err);

static TreeNode_t* ReadHeader(size_t* pos, char* buffer, TreeErr_t* err){
    assert(buffer);
    assert(pos);
    assert(err);

    if(*err) return NULL;

    int len = 0;
    char buffer_var[BUFFER_LEN] = {};
    sscanf(buffer + *pos, " \"%[^\"]\"%n", buffer_var, &len);

    TreeNode_t* node = NULL;

    node = parse_variable(buffer_var);

    if(!node) node = parse_func_call(buffer_var);

    if(!node) node = parse_function(buffer_var);

    if(!node) node = parse_function_main(buffer_var);

    if(!node) node = parse_digit(buffer_var);

    if(!node) node = parse_op(buffer_var, err);

    if(!node) node = parse_std_func(buffer_var, err);

    (*pos) += len;

    return node;
}

static TreeNode_t* parse_digit(const char* buffer){ 
    if(isdigit(buffer[0]) || (buffer[0] == '-' && isdigit(buffer[1]))){
        size_t line = 0;
        ssize_t position_in_line = 0;
        double val =  0;
        sscanf(buffer, "%lg line %zu pos %zd", &val, &line, &position_in_line); 
        return NodeCtor(CONST, (TreeElem_t){.const_value = val}, NULL, NULL, NULL, NULL, line, position_in_line);
    }
    return NULL;
}

static TreeNode_t* parse_op(const char* buffer, TreeErr_t* err){ 
    if(!strncmp("OP", buffer, 2)){
        size_t line = 0;
        ssize_t position_in_line = 0;
        size_t idx = 0;
        static size_t num_of_op = sizeof(OPERATORS_INFO) / sizeof(op_info);
        sscanf(buffer, "OP %zu line %zu pos %zd", &idx, &line, &position_in_line); 
        if(idx >= num_of_op){
            *err = OP_OUT_OF_RANGE;
            return NULL;
        }
        return NodeCtor(OPERATOR, (TreeElem_t){.op = OPERATORS_INFO[idx].op}, NULL, NULL, NULL, NULL, line, position_in_line);
    }
    return NULL;
}

static TreeNode_t* parse_std_func(const char* buffer, TreeErr_t* err){ 
    if(!strncmp("STD", buffer, 3)){
        size_t line = 0;
        ssize_t position_in_line = 0;
        size_t idx = 0;
        static size_t num_of_std_func = sizeof(FUNC_INFO) / sizeof(std_func_info);
        sscanf(buffer, "STD %zu line %zu pos %zd", &idx, &line, &position_in_line);
        if(idx >= num_of_std_func){
            *err = FUNC_OUT_OF_RANGE;
            return NULL;
        }
        else if(FUNC_INFO[idx].is_void){
            return NodeCtor(FUNCTION_STANDART_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL, NULL, line, position_in_line);
        }
        return NodeCtor(FUNCTION_STANDART_NON_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL, NULL, line, position_in_line);
    }
    return NULL;
}