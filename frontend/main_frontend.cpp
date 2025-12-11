#include "io/read_program.h"
#include "tokenizing/tokenize.h"
#include "syntax_parse/make_tokens_tree.h"
#include "io/put_tree_to_file.h"


int main(){
    char* buffer = read_file_to_string_array("tests/test6.txt");
    if(!buffer) return 0;

    Tokens_t* tokens = TokenizeInput(buffer);
    if(!tokens) return 0;

    // DEBUG FOR TOKENS
    for(int i = 0; i < tokens->first_free_place; i++){
        tree_dump_func(tokens->node_arr[i], __FILE__, __func__, __LINE__, tokens->mtk, "%d pos", i);
    }

    Tokens_t* tokens_copy = TokensCtor(10);
    TreeHead_t* head = MakeLangExprTokens(tokens, tokens_copy);
    if(!head) return 0;

    TreeErr_t err = NO_MISTAKE;
    PutTreeToFile("backend/tree.txt", head->root, head, tokens->mtk, &err);

    TokensDtor(tokens);
    TokensDtor(tokens_copy);
    return 0;
}