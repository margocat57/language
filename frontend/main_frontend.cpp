#include <stdlib.h>
#include "io/read_program.h"
#include "tokenizing/tokenize.h"
#include "syntax_parse/make_tokens_tree.h"
#include "io/put_tree_to_file.h"


int main(){
    DEBUG_TREE(
    if(find_operators_info_mistake() || find_functions_info_mistake()){
        return 0;
    })

    /*
    size_t num = sizeof(OPERATORS_INFO) / sizeof(op_info);
    for(size_t i = 1; i < num; i++){
        fprintf(stderr, "[idx] = %zu\t op_num = %d\t op_debug = %s\n", i, OPERATORS_INFO[i].op, OPERATORS_INFO[i].name_for_graph_dump);
    }
    */

    char* buffer = read_file_to_string_array("tests/test_if0_one.uccello");
    if(!buffer) return 0;

    Tokens_t* tokens = TokenizeInput(buffer);
    if(!tokens) return 0;

    // DEBUG FOR TOKENS
    /*
    for(int i = 0; i < tokens->num_of_nodes; i++){
        tree_dump_func(tokens->node_arr[i], __FILE__, __func__, __LINE__, "%d pos", i);
    }
    */

    Tokens_t* tokens_copy = TokensCtor(10);
    TreeHead_t* head = MakeLangExprTokens(tokens, tokens_copy);
    if(!head) return 0;

    TreeErr_t err = NO_MISTAKE;
    PutTreeToFile("middleend/tree_file/tree.txt", head, &err);

    free(head);
    TokensDtor(tokens);
    TokensDtor(tokens_copy);
    return 0;
}