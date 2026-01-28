#include <stdio.h>
#include "../backend/make_backend_tree/read_tree_from_file/make_tree.h"
#include "../tree/tree_func.h"
#include "optimization/tree_optimize.h"
#include "../frontend/io/put_tree_to_file.h"
#include "../debug_output/graphviz_dump.h"

int main(){
    
    TreeHead_t* head = MakeBackendTree("middleend/tree_file/tree.txt");
    TreeErr_t err = NO_MISTAKE;
    TreeOptimize(&(head->root), &err);
    if(err){
        TreeDel(head);
        return 0;
    }

    tree_dump_func(head->root, __FILE__, __func__, __LINE__, "After optimization");

    PutTreeToFile("backend/make_backend_tree/tree_file/tree.txt", head, &err);
    if(err){
        TreeDel(head);
        return 0;
    }

    TreeDel(head);

    return 0;
}