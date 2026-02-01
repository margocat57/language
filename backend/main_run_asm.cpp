#include <stdio.h>
#include "Processor-and-assembler/assembler_task/file_work.h"
#include "Processor-and-assembler/assembler_task/parsing_str.h"
#include "Processor-and-assembler/assembler_task/assembler_struct.h"
#include "Processor-and-assembler/processor_task/stack_proc_backend/hash.h"
#include "Processor-and-assembler/processor_task/parse_asm_from_file.h"
#include "Processor-and-assembler/processor_task/do_instructions.h"
#include "Processor-and-assembler/processor_task/processor.h"
#include "Processor-and-assembler/cmd_info/cmd_info.h"

int main(){
    // verifying table with commands ----------------------------------
    DEBUG_ASM_PROC(
    comands_info_mistake_t error = find_functions_info_mistake();
    if(error){
        fprintf(stderr, "error %zu in table with all processor and assembler info - can't work", error);
        return 0;
    }
    )

    // making bytecode ------------------------------------
    assembler assembl = asm_init("backend/Processor-and-assembler/assembler_task/expr.txt");
    if(parser(&assembl)){
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