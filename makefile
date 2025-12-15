.PHONY: all clean run_leak_check_front run_leak_check_back 
# эти цели не являются файлами выполняй их даже если соотв файлы существуют
all: frontend backend
# когда запускаем make без цели, то выполняем первую цель после all, то есть записи make stack make all и make эквивалентны

COMP=clang++


CFLAGS_DEBUG = -D_DEBUG -DDEBUG_SMALL_TREE
COMMON_CFLAGS = -DASSEMBLER -DPROCESSOR -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions -g -fsanitize=address,leak,undefined    
LDFLAGS = -fsanitize=address,leak,undefined
CFLAGS_ASM = -DASSEMBLER -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions  -g -fsanitize=address,leak,undefined 
CFLAGS_PROC = -DPROCESSOR -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions  -g -fsanitize=address,leak,undefined 

ifdef DEBUG_TREE
    CFLAGS = $(COMMON_CFLAGS) $(CFLAGS_DEBUG)
else
    CFLAGS = $(COMMON_CFLAGS)
endif

# FRONTEND ------------------------------------------------------------------

frontend/common/tokens.o: frontend/common/tokens.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

tree/tree_func.o: tree/tree_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/include/operators_func.o: frontend/include/operators_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/debug_output_frontend/graphviz_dump.o: frontend/debug_output_frontend/graphviz_dump.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/debug_output_frontend/print_node.o: frontend/debug_output_frontend/print_node.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/io/put_tree_to_file.o: frontend/io/put_tree_to_file.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/io/read_program.o: frontend/io/read_program.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/syntax_parse/make_tokens_tree.o: frontend/syntax_parse/make_tokens_tree.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/tokenizing/tokenize.o: frontend/tokenizing/tokenize.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

stack/hash.o: stack/hash.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

stack/log.o: stack/log.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

stack/my_assert.o: stack/my_assert.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

stack/stack_func.o: stack/stack_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/nametables/nametable.o: frontend/nametables/nametable.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/nametables/table_of_nametable.o: frontend/nametables/table_of_nametable.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend_lang: frontend/main_frontend.o frontend/common/tokens.o tree/tree_func.o frontend/debug_output_frontend/graphviz_dump.o frontend/debug_output_frontend/print_node.o frontend/io/put_tree_to_file.o frontend/io/read_program.o frontend/syntax_parse/make_tokens_tree.o frontend/tokenizing/tokenize.o frontend/include/operators_func.o stack/hash.o stack/log.o stack/my_assert.o stack/stack_func.o frontend/nametables/nametable.o  frontend/nametables/table_of_nametable.o
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_front: frontend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./frontend_lang


# BACKEND ------------------------------------------------------------------

tree/tree_func.o: tree/tree_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/main_backend.o: backend/main_backend.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/Processor-and-assembler/assembler_task/file_work.o: backend/Processor-and-assembler/assembler_task/file_work.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

backend/Processor-and-assembler/assembler_task/parsing_str.o: backend/Processor-and-assembler/assembler_task/parsing_str.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

backend/Processor-and-assembler/assembler_task/assembler_struct.o: backend/Processor-and-assembler/assembler_task/assembler_struct.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

backend/Processor-and-assembler/assembler_task/metki.o: backend/Processor-and-assembler/assembler_task/metki.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

backend/Processor-and-assembler/processor_task/parse_asm_from_file.o: backend/Processor-and-assembler/processor_task/parse_asm_from_file.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_PROC)

backend/Processor-and-assembler/processor_task/processor.o: backend/Processor-and-assembler/processor_task/processor.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_PROC)

backend/Processor-and-assembler/processor_task/do_instructions.o: backend/Processor-and-assembler/processor_task/do_instructions.cpp
	$(COMP) -c $< -o $@ $(CFLAGS_PROC)

backend/metki_table/metki_table.o: backend/metki_table/metki_table.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/translate_to_asm/translator.o: backend/translate_to_asm/translator.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/make_backend_tree/make_tree.o: backend/make_backend_tree/make_tree.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/make_backend_tree/debug_output_backend/graphviz_dump.o: backend/make_backend_tree/debug_output_backend/graphviz_dump.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend/make_backend_tree/debug_output_backend/print_node.o: backend/make_backend_tree/debug_output_backend/print_node.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

backend_lang: tree/tree_func.o backend/main_backend.o backend/Processor-and-assembler/assembler_task/file_work.o backend/Processor-and-assembler/assembler_task/parsing_str.o backend/Processor-and-assembler/assembler_task/assembler_struct.o backend/Processor-and-assembler/assembler_task/metki.o backend/Processor-and-assembler/processor_task/parse_asm_from_file.o backend/Processor-and-assembler/processor_task/processor.o backend/Processor-and-assembler/processor_task/do_instructions.o  backend/metki_table/metki_table.o backend/translate_to_asm/translator.o  backend/make_backend_tree/make_tree.o backend/make_backend_tree/debug_output_backend/graphviz_dump.o backend/make_backend_tree/debug_output_backend/print_node.o stack/hash.o stack/log.o stack/my_assert.o stack/stack_func.o
	$(COMP) -o $@ $^ $(LDFLAGS)


run_leak_check_back: backend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./backend_lang


clean:
	rm -f frontend_lang frontend/*.o frontend/common/*.o frontend/debug_output_frontend/*.o frontend/io/*.o frontend/nametables/*.o frontend/syntax_parse/*.o frontend/tokenizing/*.o frontend/debug_output_frontend/images/*.dot frontend/debug_output_frontend/images/*.svg frontend/include/*.o stack/*.o tree/*.o backend/*.o backend/Processor-and-assembler/assembler_task/*.o backend/Processor-and-assembler/processor_task/*.o backend/metki_table/*.o backend_lang backend/translate_to_asm/*.o backend/make_backend_tree/*.o backend/make_backend_tree/debug_output_backend/*.o backend/make_backend_tree/debug_output_backend/images/*.dot backend/make_backend_tree/debug_output_backend/images/*.svg