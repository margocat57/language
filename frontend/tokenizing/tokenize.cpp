#include "tokenize.h"
#include "../include/operators_func.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

const size_t MAX_SIZE_BUFFER = 256;

static void skip_space(const char* str, size_t* pos, size_t* slash_n_count, size_t* num_of_symb_above);

static bool FindOperators(Tokens_t* tokens,  const char* buffer, size_t* pos);

static bool Tokenize_FUNC(Tokens_t* tokens, const char* buffer, size_t* pos);

static bool Tokenize_FUNC_MAIN(Tokens_t* tokens, const char* buffer, size_t* pos);

static bool Tokenize_Decimal(Tokens_t* tokens, const char* buffer, size_t* pos);

static bool Tokenize_Variable(Tokens_t* tokens, const char* buffer, size_t* pos);

static bool Find_And_Skip_Comments(Tokens_t* tokens, const char* buffer, size_t* pos);

Tokens_t* TokenizeInput(const char* buffer){
    Tokens_t* tokens = TokensCtor(10);
    size_t pos = 0;
    size_t slash_n_count = 0;
    size_t num_of_symb_above = 0;
    bool is_token_found = false;

    while(true){
        is_token_found = false;
        skip_space(buffer, &pos, &slash_n_count, &num_of_symb_above);

        if(!strncmp(buffer + pos, "Fottiti!", sizeof("Fottiti!") - 1)){
            break;
        }

        if(Find_And_Skip_Comments(tokens, buffer, &pos))      continue;

        else if(FindOperators(tokens, buffer, &pos))          continue; 

        else if(Tokenize_FUNC(tokens, buffer, &pos))          continue; 

        else if(Tokenize_FUNC_MAIN(tokens, buffer, &pos))     continue; 

        else if(Tokenize_Decimal(tokens, buffer, &pos))       continue; 

        else if(Tokenize_Variable(tokens, buffer, &pos))      continue;

        else{
            TokensDtor(tokens);
            fprintf(stderr, "Syntax error incorr symbol %c line %zu pos %zu\n", buffer[pos], slash_n_count, pos - num_of_symb_above);
            return NULL;
        }
    }
    free((char*)buffer); // временная мера
    return tokens;
}

static void skip_space(const char* str, size_t* pos, size_t* slash_n_count, size_t* num_of_symb_above){
    char ch = str[(*pos)];
    while(isspace(ch) && ch != '\0'){
        if(str[(*pos)] == '\n'){
            (*slash_n_count)++;
            *num_of_symb_above = *pos - 1;
        }
        (*pos)++;
        ch = str[(*pos)];
    }
}

static bool Tokenize_FUNC(Tokens_t* tokens, const char* buffer, size_t* pos){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(!strncmp(buffer + *pos, "Che_cazzo", sizeof("Che_cazzo") - 1)){
        sscanf(buffer + *pos, " %[^ {]%n", buffer_var, &num_of_symb);
        *pos += num_of_symb;
        TokensAddElem(NodeCtor(FUNCTION, {} , NULL, NULL, NULL, strdup(buffer_var)), tokens);
        return true;
    }
    return false;
}

static bool Tokenize_FUNC_MAIN(Tokens_t* tokens, const char* buffer, size_t* pos){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(!strncmp(buffer + *pos, "Che_grande_cazzo", sizeof("Che_grande_cazzo") - 1)){
        sscanf(buffer + *pos, " %[^ {]%n", buffer_var, &num_of_symb);
        *pos += num_of_symb;
        TokensAddElem(NodeCtor(FUNCTION_MAIN, {} , NULL, NULL, NULL, strdup(buffer_var)), tokens);
        return true;
    }
    return false;
}

static bool FindOperators(Tokens_t* tokens, const char* buffer, size_t* pos){
    size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info);
    for(size_t idx = 1; idx < num_of_operators; idx++){
        if(!OPERATORS_INFO[idx].op_name_in_code){
            continue;
        }
        if(!strncmp(buffer + *pos, OPERATORS_INFO[idx].op_name_in_code, OPERATORS_INFO[idx].num_of_symb_code)){
            TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OPERATORS_INFO[idx].op}, NULL, NULL, NULL), tokens);
            *pos += OPERATORS_INFO[idx].num_of_symb_code;
            return true;
        }
    }
    return false;
}

static bool Tokenize_Decimal(Tokens_t* tokens, const char* buffer, size_t* pos){
    int val = 0;
    int num_of_symb = 0;
    if(isdigit(buffer[*pos])){
        char* endptr = NULL;
        val = strtol(buffer + *pos, &endptr, 10);
        *pos +=  endptr - (buffer + *pos);
        TokensAddElem(NodeCtor(CONST, (TreeElem_t){.const_value = val}, NULL, NULL, NULL), tokens);
        return true;
    }
    return false;
}

static bool Tokenize_Variable(Tokens_t* tokens, const char* buffer, size_t* pos){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(isalpha(buffer[*pos])){
        sscanf(buffer + *pos, " %s%n", buffer_var, &num_of_symb);
        *pos += num_of_symb;
        TokensAddElem(NodeCtor(VARIABLE, {}, NULL, NULL, NULL, strdup(buffer_var)), tokens);
        return true;
    }
    return false;
}

static bool Find_And_Skip_Comments(Tokens_t* tokens, const char* buffer, size_t* pos){
    if(!strncmp(buffer + *pos, "STA_CAGATA", sizeof("STA_CAGATA") - 1)){
        *pos += sizeof("STA_CAGATA") - 1;
        while(true){
            if(!strncmp(buffer + *pos, "NON_PUBBLICARLA", sizeof("NON_PUBBLICARLA") - 1)){
                break;
            }
            (*pos)++;
        }
        *pos += sizeof("NON_PUBBLICARLA") - 1;
        return true;
    }
    return false;
}