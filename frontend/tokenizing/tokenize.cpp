#include "tokenize.h"
#include "../../include/operators_func.h"
#include "../../include/standart_func.h"
#include "../io/read_program.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <math.h> 

const size_t MAX_SIZE_BUFFER = 256;
uint8_t DIGIT_FOR_SKIP_SLASH_N = 2;

static void skip_space(const char* str, size_t* pos, size_t* slash_n_count, ssize_t* num_of_symb_above);

static bool FindOperators(Tokens_t* tokens,  const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool FindStandartFunc(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool Tokenize_FUNC(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool Tokenize_FUNC_MAIN(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool Tokenize_Decimal(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool Tokenize_Variable(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

static bool Find_And_Skip_Comments(const char* buffer, size_t* pos, size_t* slash_n_count, ssize_t* num_of_symb_above);

static void OutputErrorMsg(char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above);

Tokens_t* TokenizeInput(char* buffer){
    Tokens_t* tokens = TokensCtor(10);
    size_t pos = 0;
    size_t slash_n_count = 0;
    ssize_t num_of_symb_above = 0;
    bool is_token_found = false;

    while(true){
        skip_space(buffer, &pos, &slash_n_count, &num_of_symb_above);

        if(!strncmp(buffer + pos, "Fottiti!", sizeof("Fottiti!") - 1)){
            break;
        }

        if(Find_And_Skip_Comments(buffer, &pos, &slash_n_count, &num_of_symb_above))            continue;

        else if(FindOperators(tokens, buffer, &pos, slash_n_count, num_of_symb_above))          continue; 

        else if(FindStandartFunc(tokens, buffer, &pos, slash_n_count, num_of_symb_above))       continue;

        else if(Tokenize_FUNC(tokens, buffer, &pos, slash_n_count, num_of_symb_above))          continue; 

        else if(Tokenize_FUNC_MAIN(tokens, buffer, &pos, slash_n_count, num_of_symb_above))     continue; 

        else if(Tokenize_Decimal(tokens, buffer, &pos, slash_n_count, num_of_symb_above))       continue; 

        else if(Tokenize_Variable(tokens, buffer, &pos, slash_n_count, num_of_symb_above))      continue;

        else{
            TokensDtor(tokens);
            OutputErrorMsg(buffer, &pos, slash_n_count, num_of_symb_above);
            return NULL;
        }
    }
    buffer_free(buffer);
    return tokens;
}

static void skip_space(const char* str, size_t* pos, size_t* slash_n_count, ssize_t* num_of_symb_above){
    char ch = str[(*pos)];
    while(isspace(ch) && ch != '\0'){
        if(str[(*pos)] == '\n'){
            (*slash_n_count)++;
            if(*pos!= 0) *num_of_symb_above = *pos - 1;
        }
        (*pos)++;
        ch = str[(*pos)];
    }
}

static bool Tokenize_FUNC(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(!strncmp(buffer + *pos, "Che_cazzo", sizeof("Che_cazzo") - 1)){
        ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
        if (pos_in_str < 0) pos_in_str = 0;

        sscanf(buffer + *pos, " %[^ {]%n", buffer_var, &num_of_symb);
        if(num_of_symb < 0) return false;

        TokensAddElem(NodeCtor(FUNCTION, {} , NULL, NULL, NULL, strdup(buffer_var), slash_n_count + 1, pos_in_str), tokens);
        *pos += num_of_symb;
        return true;
    }
    return false;
}

static bool Tokenize_FUNC_MAIN(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(!strncmp(buffer + *pos, "Che_grande_cazzo", sizeof("Che_grande_cazzo") - 1)){
        ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
        if (pos_in_str < 0) pos_in_str = 0;

        sscanf(buffer + *pos, " %[^ {]%n", buffer_var, &num_of_symb);
        if(num_of_symb < 0) return false;

        TokensAddElem(NodeCtor(FUNCTION_MAIN, {} , NULL, NULL, NULL, strdup(buffer_var), slash_n_count + 1, pos_in_str), tokens);
        *pos += num_of_symb;
        return true;
    }
    return false;
}

static bool FindOperators(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    size_t num_of_operators = sizeof(OPERATORS_INFO) / sizeof(op_info);
    for(size_t idx = 1; idx < num_of_operators; idx++){
        if(!OPERATORS_INFO[idx].op_name_in_code){
            continue;
        }
        if(!strncmp(buffer + *pos, OPERATORS_INFO[idx].op_name_in_code, OPERATORS_INFO[idx].num_of_symb_code)){
            ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
            if (pos_in_str < 0) pos_in_str = 0;

            TokensAddElem(NodeCtor(OPERATOR, (TreeElem_t){.op = OPERATORS_INFO[idx].op}, NULL, NULL, NULL, NULL, slash_n_count + 1, pos_in_str), tokens);
            *pos += OPERATORS_INFO[idx].num_of_symb_code;
            return true;
        }
    }
    return false;
}

static bool FindStandartFunc(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    size_t num_of_func = sizeof(FUNC_INFO) / sizeof(std_func_info);
    for(size_t idx = 0; idx < num_of_func; idx++){
        if(!FUNC_INFO[idx].func_name_in_code){
            continue;
        }
        if(!strncmp(buffer + *pos, FUNC_INFO[idx].func_name_in_code, FUNC_INFO[idx].num_of_symb_code)){
            ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
            if (pos_in_str < 0) pos_in_str = 0;

            if(FUNC_INFO[idx].is_void){
                TokensAddElem(NodeCtor(FUNCTION_STANDART_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL, NULL, slash_n_count + 1, pos_in_str), tokens);
            }
            else{
                TokensAddElem(NodeCtor(FUNCTION_STANDART_NON_VOID, (TreeElem_t){.stdlib_func = FUNC_INFO[idx].function}, NULL, NULL, NULL, NULL, slash_n_count + 1, pos_in_str), tokens);
            }
            *pos += FUNC_INFO[idx].num_of_symb_code;
            return true;
        }
    }
    return false;
}

static bool Tokenize_Decimal(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    double val = 0;
    if(isdigit(buffer[*pos]) || (buffer[*pos] == '-' && isdigit(buffer[*pos + 1]))){
        ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
        if (pos_in_str < 0) pos_in_str = 0;

        char* endptr = NULL;
        val = strtod(buffer + *pos, &endptr);

        TokensAddElem(NodeCtor(CONST, (TreeElem_t){.const_value = val}, NULL, NULL, NULL, NULL, slash_n_count + 1, pos_in_str, endptr - (buffer + *pos)), tokens);
        *pos +=  endptr - (buffer + *pos);
        return true;
    }
    return false;
}

static bool Tokenize_Variable(Tokens_t* tokens, const char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    char buffer_var[MAX_SIZE_BUFFER] = {};
    int num_of_symb = 0;
    if(isalpha(buffer[*pos])){
        ssize_t pos_in_str = *pos - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N;
        if (pos_in_str < 0) pos_in_str = 0;

        sscanf(buffer + *pos, " %s%n", buffer_var, &num_of_symb);
        TokensAddElem(NodeCtor(VARIABLE, {}, NULL, NULL, NULL, strdup(buffer_var), slash_n_count + 1, pos_in_str), tokens);

        *pos += num_of_symb;
        return true;
    }
    return false;
}

static bool Find_And_Skip_Comments(const char* buffer, size_t* pos, size_t* slash_n_count, ssize_t* num_of_symb_above){
    if(!strncmp(buffer + *pos, "STA_CAGATA", sizeof("STA_CAGATA") - 1)){
        *pos += sizeof("STA_CAGATA") - 1;
        while(true){
            if(!strncmp(buffer + *pos, "NON_PUBBLICARLA", sizeof("NON_PUBBLICARLA") - 1)){
                break;
            }
            (*pos)++;
            skip_space(buffer, pos, slash_n_count, num_of_symb_above);
        }
        *pos += sizeof("NON_PUBBLICARLA") - 1;
        return true;
    }
    return false;
}

//--------------------------------------------------------------
// Errors aka GCC output

#define RED                        "\033[1;31m"
#define GREEN                      "\033[0;32m"
#define RESET                      "\033[0m"

static size_t count_digits_log(size_t num);

static void OutputErrorMsg(char* buffer, size_t* pos, size_t slash_n_count, ssize_t num_of_symb_above){
    for(size_t current_pos = *pos ; buffer[current_pos] != '\0'; current_pos++){
        if(buffer[current_pos] == '\n'){
            buffer[current_pos] = '\0';
            break;
        }
    }

    size_t width = count_digits_log(slash_n_count + 1);
    fprintf(stderr, RED "error: " RESET "Incorrect symbol %c\n", buffer[*pos]);
    if((*pos) < num_of_symb_above + DIGIT_FOR_SKIP_SLASH_N){
        fprintf(stderr, "%*zu | %s\n", width, slash_n_count, buffer + (*pos));
        return;
    }
    fprintf(stderr, "%*zu |%s\n", width, slash_n_count + 1, buffer + num_of_symb_above + DIGIT_FOR_SKIP_SLASH_N);
    fprintf(stderr, "%*s |", width, ""); 
    fprintf(stderr, "%*s" GREEN "^" RESET"\n", (*pos) - num_of_symb_above - DIGIT_FOR_SKIP_SLASH_N, "");
}

static size_t count_digits_log(size_t num){
    if (num == 0) return 1;
    return (size_t)log10(num) + 1;
}

#undef RED   
#undef GREEN                      
#undef RESET 
//--------------------------------------------------------------