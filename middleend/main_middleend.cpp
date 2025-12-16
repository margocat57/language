#include <stdio.h>
#include "../backend/make_backend_tree/make_tree.h"
#include "../tree/tree_func.h"
#include "optimization/tree_optimize.h"
#include "put_tree_to_file_middle/put_tree_to_file.h"

int main(){
    
    TreeHead_t* head = MakeBackendTree("middleend/tree_file/tree.txt");
    TreeErr_t err = NO_MISTAKE;
    TreeOptimize(&(head->root), &err);
    if(err){
        TreeDel(head);
        return 0;
    }

    PutTreeToFileMiddle("backend/make_backend_tree/tree_file/tree.txt", head, &err);
    if(err){
        TreeDel(head);
        return 0;
    }

    TreeDel(head);

    return 0;
}