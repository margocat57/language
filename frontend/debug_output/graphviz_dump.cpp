#include "../common/tree_func.h"
#include "graphviz_dump.h"
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "../nametables/table_of_nametable.h"

#define CHECK_AND_RET_DUMP(bad_condition, msg)\
    if(bad_condition){ \
        fprintf(stderr, msg); \
        return; \
    } \

static filenames_for_dump filename_ctor();

static int check_and_dump_system_work(int sys_res);

static void generate_dot_file(const TreeNode_t* node, const char* dot_filename, name_table* mtk);

static void generate_svg_file(const filenames_for_dump* dump);

static void tree_dump_html(const char* img, const char* debug_msg, const char *file, const char *func, int line, va_list args);  

void tree_dump_func(const TreeNode_t* node, const char *file, const char *func,  int line, name_table* mtk, const char* debug_msg, ...){
    assert(file);

    filenames_for_dump dump = filename_ctor();
    generate_dot_file(node, dump.dot_filename, mtk);
    generate_svg_file(&dump);
    if(dump.svg_filename){
        va_list args = {};
        va_start(args, line);
        tree_dump_html(dump.svg_filename, debug_msg, file, func, line, args);
        free(dump.svg_filename);
        va_end(args);
    }
    if(dump.dot_filename){
        free(dump.dot_filename);
    }
}

static void tree_dump_html(const char* img, const char* debug_msg, const char *file, const char *func, int line,va_list args) {
    static int launch_num = 0;
    FILE* html_output = NULL;
    
    if(launch_num == 0){
        html_output = fopen(LOG_FILE, "w");
        CHECK_AND_RET_DUMP(!html_output, "Can't open html file\n");
        launch_num++;
        fprintf(html_output, "<pre style=\"background-color: #FFFAFA; color: #000000;\">");
        fprintf(html_output, "<p style=\"font-size: 50px; text-align: center;\"> TREE DUMP\n");
    }
    else{
        html_output = fopen(LOG_FILE, "a+");
        CHECK_AND_RET_DUMP(!html_output, "Can't open html file\n");
    }
    fprintf(html_output, "<p style=\"font-size: 20px; \">Dump was called at %s function %s line %d\n", file, func, line);
    fprintf(html_output, "<p style=\"font-size: 17.5px; color: #bb0d12;\">");
    vfprintf(html_output, debug_msg, args);
    fprintf(html_output, "\n");

    fprintf(html_output, "<img src=\"%s\" alt=\"Tree visualization\" width=\"45%%\">\n", img);
    fprintf(html_output, "\n");
    fclose(html_output);
}

static filenames_for_dump filename_ctor(){
    filenames_for_dump dump = {};
    static int num = 0;
    num++;

    char* dot_filename = (char*)calloc(200, sizeof(char));
    if(!dot_filename){
        fprintf(stderr, "Allocation error for string with .dot filename");
        return dump;
    }

    char* svg_filename = (char*)calloc(200, sizeof(char));
    if(!svg_filename){
        fprintf(stderr, "Allocation error for string with .svg filename");
        return dump;
    }

    dump.svg_filename = svg_filename;
    dump.dot_filename = dot_filename;

    time_t now = time(NULL); 
    struct tm *t = localtime(&now); 

    if(snprintf(dump.dot_filename, 200, 
                "frontend/debug_output/images/dump%d_%04d%02d%02d_%02d%02d%02d.dot", num,
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec) == -1) {
        fprintf(stderr, "Can't generate dot filename\n");
        free(dump.svg_filename);
        return {};
    }
    
    if(snprintf(dump.svg_filename, 200, 
                "frontend/debug_output/images/dump%d_%04d%02d%02d_%02d%02d%02d.svg", num,
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec) == -1) {
        fprintf(stderr, "Can't generate svg filename\n");
        free(dump.dot_filename);
        return {};
    }
    return dump;
}

static void generate_dot_file(const TreeNode_t* node, const char* dot_filename, name_table* mtk){
    if(!dot_filename){
        fprintf(stderr, "NULL dot_filename pointer - can't work\n");
        return;
    }

    FILE* dot_file = fopen(dot_filename, "w");
    if(!dot_file){
        fprintf(stderr, "Can't open dot file\n");
        return;
    }

    fprintf(dot_file, "digraph G{\n");
    fprintf(dot_file, " rankdir=TB;\n");
    fprintf(dot_file, " charset=\"utf-8\";\n");
    // fprintf(dot_file, " splines=ortho;\n");
    fprintf(dot_file, " graph [bgcolor=\"#FFFAFA\", nodesep = 0.3, ranksep=0.8];\n");

    int rank = 1;
    PrintNode(node, dot_file, &rank, mtk);

    fprintf(dot_file,"}\n");
    fclose(dot_file);
}

static void generate_svg_file(const filenames_for_dump* dump){
    char command[200] = {0};

    if(snprintf(command, sizeof(command) ,"dot -Tsvg %s -o %s", dump->dot_filename, dump->svg_filename) == -1){
        fprintf(stderr, "Can't parse command to do\n");
        return;
    }
    int sys_res = system(command);
    check_and_dump_system_work(sys_res);
}

static int check_and_dump_system_work(int sys_res){
    if(sys_res == 0){
        return 0;
    }
    if(sys_res == -1){
        fprintf(stderr, "Cannot start to do command\n");
        return -1;
    }
    if(WIFEXITED(sys_res)){
        int exit_stat = WEXITSTATUS(sys_res);
        fprintf(stderr, "Creating svg file finished with mistake %d\n", exit_stat);
        return exit_stat;
    }
    return 0;
}