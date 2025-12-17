#include <stdio.h>
#include "make_backend_tree/read_tree_from_file/make_tree.h"
#include "make_backend_tree/find_rbp_shift/calcul_rbp/calcul_rbp.h"
#include "translate_to_asm/translator.h"

int main(){

    // making asm code -----------------------------------
    TreeHead_t* head = MakeBackendTree("backend/make_backend_tree/tree_file/tree.txt");
    TreeErr_t err = NO_MISTAKE;
    CalculRpbShift(head, &err);
    if(err){
        TreeDel(head);
        return 0;
    } 
    CreateAsmCode("backend/Processor-and-assembler/assembler_task/expr.txt", head, &err);

    TreeDel(head);
    return 0;

}