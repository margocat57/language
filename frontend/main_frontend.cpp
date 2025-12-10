#include "io/read_program.h"
#include "tokenizing/tokenize.h"
#include "syntax_parse/make_tokens_tree.h"
#include "io/put_tree_to_file.h"

// проблема - утечка node_sp!

int main(){
    char* buffer = read_file_to_string_array("tests/test2.txt");
    Tokens_t* tokens = TokenizeInput(buffer);
    for(int i = 0; i < tokens->first_free_place; i++){
        tree_dump_func(tokens->node_arr[i], __FILE__, __func__, __LINE__, tokens->mtk, "%d pos", i);
    }
    TreeHead_t* head = MakeLangExprTokens(tokens);
    TreeErr_t err = NO_MISTAKE;
    PutTreeToFile("backend/tree.txt", head->root, head, tokens->mtk, &err);
    TokensDtor(tokens);
    return 0;
}