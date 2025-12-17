#ifndef GRAPHVIZ_DUMP_H
#define GRAPHVIZ_DUMP_H
#include <string.h>
#include "../tree/tree.h"

const char* const LOG_FILE = "log.htm";

struct filenames_for_dump{
    char* dot_filename;
    char* svg_filename;
};

void tree_dump_func(const TreeNode_t* node, const char *file, const char *func, int line, const char* debug_msg, ...) __attribute__ ((format (printf, 5, 6)));

#endif // GRAPHVIZ_DUMP_H