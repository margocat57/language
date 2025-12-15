#include <stdio.h>
#include "make_backend_tree/make_tree.h"
#include "translate_to_asm/translator.h"
#include "Processor-and-assembler/assembler_task/file_work.h"
#include "Processor-and-assembler/assembler_task/parsing_str.h"
#include "Processor-and-assembler/assembler_task/assembler_struct.h"
#include "../stack/hash.h"
#include "Processor-and-assembler/processor_task/parse_asm_from_file.h"
#include "Processor-and-assembler/processor_task/do_instructions.h"
#include "Processor-and-assembler/processor_task/processor.h"

int main(){

    // making asm code -----------------------------------
    TreeHead_t* head = MakeBackendTree("backend/make_backend_tree/tree_file/tree.txt");
    TreeErr_t err = NO_MISTAKE;
    CreateAsmCode("backend/Processor-and-assembler/assembler_task/expr.txt", head, &err);
    if(err) return 0;

    // making bytecode ------------------------------------
    assembler assembl = asm_init("backend/Processor-and-assembler/assembler_task/expr.txt");
    if (parser(&assembl)){
        free_asm(&assembl);
        return 0;
    }
    asm_dump(&assembl);
    put_buffer_to_file("backend/Processor-and-assembler/assembler_task/ass.bin", &assembl);
    free_asm(&assembl);

    // running bytecode ----------------------------------
    processor baikal = {};
    baikal = init("backend/Processor-and-assembler/assembler_task/ass.bin");
    stack_err_bytes err_main = do_processor_comands(&baikal);

    processor_free(&baikal);

    return 0;
}