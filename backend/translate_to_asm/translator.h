#ifndef TRASLATOR_H
#define TRASLATOR_H
#include "../../tree/tree_func.h"

struct op_counters{
    size_t if_count;
    size_t while_count;
    size_t else_count;
    size_t param_count;
    size_t first_free;
};

void CreateAsmCode(const char* file_name, const TreeHead_t* head, TreeErr_t* err);

void CreateCommaAsm       (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateReturnAsm      (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateDrawStdFuncAsm (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateOpenBrAsm      (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateInitAssAsm     (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateIfAsm          (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateElseAsm        (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateWhileAsm       (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateOutputAsm      (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateInputAsm       (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateExitAsm        (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateAddAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateSubAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateDivAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateMulAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateEqAsm          (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateGeAsm          (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateLeAsm          (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateGeqAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateLeqAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateDegAsm         (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateRamDumpAsm     (FILE *file, TreeNode_t *node, op_counters* counters,   TreeErr_t* err);
void CreateBreakAsm       (FILE *file, TreeNode_t *node, op_counters* counters, TreeErr_t* err);

#endif //TRASLATOR_H