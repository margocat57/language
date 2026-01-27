#include <assert.h>
#include "print_node.h"
#include "../tree/tree_func.h"
#include "../include/operators_func.h"
#include "../include/standart_func.h"

//----------------------------------------------------------------
// Dumping tree

static void PrintNodeConnect(const TreeNode_t* node, const TreeNode_t* node_child, FILE* dot_file, int* rank);

TreeErr_t PrintNode(const TreeNode_t* node, FILE* dot_file, int* rank){
    assert(node);
    assert(rank);
    assert(dot_file);

    if(node->left){
        PrintNodeConnect(node, node->left, dot_file, rank);
        CHECK_AND_RET_TREEERR(PrintNode(node->left, dot_file, rank))
    }

    if(node->type == OPERATOR){
        size_t arr_num_of_elem = sizeof(OPERATORS_INFO) / sizeof(op_info);
        if(node->data.op >= arr_num_of_elem){
            fprintf(stderr, "out of range %d %zu\n", node->data.op, arr_num_of_elem);
            return INCORR_OPERATOR;
        }
        fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#00FFFF\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = OPERATOR} | {val = %d(%s)} | {L | R }} \"];\n", node, *rank, node->data.op, OPERATORS_INFO[node->data.op].name_for_graph_dump);
    }
    else if(node->type == FUNCTION_STANDART_NON_VOID || node->type == FUNCTION_STANDART_VOID){
        size_t arr_num_of_elem = sizeof(FUNC_INFO) / sizeof(std_func_info);
        if(node->data.stdlib_func >= arr_num_of_elem){
            fprintf(stderr, "out of range %d %zu\n", node->data.op, arr_num_of_elem);
            return INCORR_OPERATOR;
        }
        fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#F19CBB\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = STD_FUNC} | {val = %d(%s)} | {L | R }} \"];\n", node, *rank, node->data.stdlib_func, FUNC_INFO[node->data.stdlib_func].name_for_graph_dump);
    }
    else{
        if(node->type == CONST){
            fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#98FB98\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = CONST_VAR} | {val = %lg} | {0 | 0}} \"];\n", node, *rank, node->data.const_value);
        }
        else if(node->type == VARIABLE){
            if(node->var_func_name){
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#DAA520\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = VARIABLE} | {val = %s} | {0 | 0}} \"];\n", node, *rank, node->var_func_name);
            }
            else{
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#DAA520\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = VARIABLE} | {val = %zu} | {0 | 0}} \"];\n", node, *rank, node->data.var_code);
            }
        }
        else if(node->type == FUNCTION || node->type == FUNCTION_MAIN){
            if(node->var_func_name){
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#a7a7f2\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = FUNCTION} | {val = %s} | {0 | 0}} \"];\n", node, *rank, node->var_func_name);
            }
            else{
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#a7a7f2\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = FUNCTION} | {val = %zu} | {0 | 0}} \"];\n", node, *rank, node->data.var_code);
            }
        }
        else if(node->type == FUNC_CALL){
            if(node->var_func_name){
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#a7a7f2\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = FUNCTION_CALL} | {val = %s} | {0 | 0}} \"];\n", node, *rank, node->var_func_name);
            }
            else{
                fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#a7a7f2\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{type = FUNCTION_CALL} | {val = %zu} | {0 | 0}} \"];\n", node, *rank, node->data.var_code);
            }
        }
    }

    if(node->right){
        PrintNodeConnect(node, node->right, dot_file, rank);
        CHECK_AND_RET_TREEERR(PrintNode(node->right, dot_file, rank))
    }
    (*rank)--;
    return NO_MISTAKE;
}

static void PrintNodeConnect(const TreeNode_t* node, const TreeNode_t* node_child, FILE* dot_file, int* rank){
    assert(rank);
    assert(node_child);

    if(node_child->parent == node){
        fprintf(dot_file, " node_%p -> node_%p[color = \"#964B00\", dir = both];\n", node, node_child); 
    }
    else{
        fprintf(dot_file, " node_%p[shape=\"Mrecord\", style=\"filled\", fillcolor=\"#ff0000\", rank=%d, color = \"#964B00\", penwidth=1.0, label=\"{{INCORRECT NODE} | {INCORR! | INCORR!}} \"];\n", node_child->parent, *rank);
        fprintf(dot_file, " node_%p -> node_%p[color = \"#0000FF\"];\n", node, node_child);
        if(node_child->parent){
            fprintf(dot_file, " node_%p -> node_%p[color = \"#FF4F00\"];\n", node_child->parent, node_child);
        }
    }
    (*rank)++;
}