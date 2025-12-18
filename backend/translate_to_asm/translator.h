#ifndef TRASLATOR_H
#define TRASLATOR_H
#include "../../tree/tree_func.h"
#include "../metki_table/metki_table.h"

void CreateAsmCode(const char* file_name, const TreeHead_t* head, TreeErr_t* err);

void CreateCommaAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateReturnAsm      (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateSpCloseFigBrAsm(FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateInitAssAsm     (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateIfAsm          (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateElseAsm        (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateWhileAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateOutputAsm      (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateInputAsm       (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateExitAsm        (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateAddAsm         (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateSubAsm         (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateDivAsm         (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateMulAsm         (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateEqAsm          (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateGeAsm          (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateLeAsm          (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateDegAsm         (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);
void CreateRamDumpAsm     (FILE *file, TreeNode_t *node, metki_for_translate* mtk, int* op_count, int* param_count,   TreeErr_t* err);

#endif //TRASLATOR_H