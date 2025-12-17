#ifndef CALCUL_RBP_H
#define CALCUL_RBP_H
#include "../../../../tree/tree_func.h"
#include "../stack_for_var_codes/stack_rbp.h"
#include "../stack_for_var_codes/stack_func_rbp.h"
#include "../nametables/nametable.h"

void CalculRpbShift(const TreeHead_t* head, TreeErr_t* err);

void CalculRpbShiftFuncCall(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShiftOther(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShiftFuncDecl(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShiftIfElseWhile(TreeNode_t* node, Stack_t* stack, name_table* nametable,  size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShiftInit(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init,  TreeErr_t* err, bool is_init);

void CalculRpbShiftVariable(TreeNode_t* node, Stack_t* stack, name_table* nametable, size_t *num_of_variables_init, TreeErr_t* err, bool is_init);

#endif //CALCUL_RBP_H