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

static TreeErr_t ReadNode(size_t* pos, char* buffer, TreeNode_t** node_to_write);

TreeHead_t* MakeBackendTree(const char *name_of_file){
    assert(name_of_file);

    char* buffer = read_file_to_string_array(name_of_file);
    if(!buffer){
        return NULL;
    }
    TreeHead_t* head = TreeCtor();
    size_t pos = 0;
    if(ReadNode(&pos, buffer, &(head->root))){
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

static TreeNode_t* ReadHeader(size_t* pos, char* buffer);

static TreeErr_t ReadNode(size_t* pos, char* buffer, TreeNode_t** node_to_write){
    assert(pos);
    assert(buffer);
    assert(node_to_write);

    skip_space(buffer, pos);
    if(buffer[(*pos)] == '('){
        (*pos)++; //skip '('
        skip_space(buffer, pos);

        if(buffer[(*pos)] != '"'){
            fprintf(stderr, "Incorr file no \"\n");
            return INCORR_FILE;
        }

        *node_to_write = ReadHeader(pos, buffer); 
        skip_space(buffer, pos);
        if(!*node_to_write) return INCORR_FILE;

        if(ReadNode(pos, buffer, &((*node_to_write)->left))){
            fprintf(stderr, "Incorr file\n");
            return INCORR_FILE;
        }

        if(ReadNode(pos, buffer, &((*node_to_write)->right))){
            fprintf(stderr, "Incorr file\n");
            return INCORR_FILE;
        }

        skip_space(buffer, pos);
        if(buffer[(*pos)] != ')'){
            fprintf(stderr, "Incorr file no )\n");
            return INCORR_FILE;
        }
        (*pos)++; //skip ')'
        skip_space(buffer, pos);
        return NO_MISTAKE;
    }
    else if(!strncmp(buffer + *pos, "nil", 3)){
        (*pos) += strlen("nil");
        *node_to_write = NULL;
        skip_space(buffer, pos);
        return NO_MISTAKE;
    }
    fprintf(stderr, "Incorr file undef symbol(%s), pos(%zu)\n", buffer + *pos, *pos);
    return INCORR_FILE;
}

//-------------------------------------------------------------------------------------------
// Reading string from file

// на будущее - дампить не по типу а число в енаме и делать свитч по числу

static TreeNode_t* ReadHeader(size_t* pos, char* buffer){
    assert(buffer);
    assert(pos);

    int len = 0;
    char buffer_var[BUFFER_LEN] = {};
    sscanf(buffer + *pos, " \"%[^\"]\"%n", buffer_var, &len);

    TreeNode_t* node = NULL;
    if(!strncmp("VAR", buffer_var, 3)){
        size_t idx = 0;
        char var_name[BUFFER_LEN] = {};
        sscanf(buffer_var, "VAR %s", var_name);
        node = NodeCtor(VARIABLE, {}, NULL, NULL, NULL, strdup(var_name));
    }
    else if(!strncmp("CALL", buffer_var, 4)){
        size_t idx = 0;
        char func_name[BUFFER_LEN] = {};
        sscanf(buffer_var, "CALL %s", func_name);
        node = NodeCtor(FUNC_CALL, {}, NULL, NULL, NULL, strdup(func_name));
    }
    else if(!strncmp("FUNC", buffer_var, 4)){
        char func_name[BUFFER_LEN] = {};
        sscanf(buffer_var, "FUNC %s", func_name);
        node = NodeCtor(FUNCTION, {}, NULL, NULL, NULL, strdup(func_name));
    }
    else if(!strncmp("MAIN", buffer_var, 4)){
        char func_name[BUFFER_LEN] = {};
        sscanf(buffer_var, "MAIN %s", func_name);
        node = NodeCtor(FUNCTION_MAIN, {}, NULL, NULL, NULL, strdup(func_name));
    }
    else if(isdigit(buffer_var[0]) || (buffer_var[0] == '-' && isdigit(buffer_var[1]))){
        double val = strtod(buffer_var, NULL);
        node = NodeCtor(CONST, (TreeElem_t){.const_value = val}, NULL, NULL, NULL);
    }
    else if(!strncmp("OP", buffer_var, 2)){
        int idx = 0;
        sscanf(buffer_var, "OP %d", &idx); //TODO проверка на допустимый индекс
        node = NodeCtor(OPERATOR, (TreeElem_t){.op = OPERATORS_INFO[idx].op}, NULL, NULL, NULL);
    }
    else if(!strncmp("STD", buffer_var, 3)){
        int idx = 0;
        sscanf(buffer_var, "STD %d", &idx);
        if(FUNC_INFO[idx].is_void){
            node = NodeCtor(FUNCTION_STANDART_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL);
        }
        else{
            node = NodeCtor(FUNCTION_STANDART_NON_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL);
        }
    }
    (*pos) += len;

    return node;
}