.PHONY: all clean run_leak_check run_debug run_with_gdb
# эти цели не являются файлами выполняй их даже если соотв файлы существуют
all: lang
# когда запускаем make без цели, то выполняем первую цель после all, то есть записи make stack make all и make эквивалентны

COMP=clang++


CFLAGS_DEBUG = -D_DEBUG -DDEBUG_SMALL_TREE
COMMON_CFLAGS = -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions -g -fsanitize=address,leak,undefined    
LDFLAGS = -fsanitize=address,leak,undefined

ifdef DEBUG_TREE
    CFLAGS = $(COMMON_CFLAGS) $(CFLAGS_DEBUG)
else
    CFLAGS = $(COMMON_CFLAGS)
endif

frontend/common/metki.o: frontend/common/metki.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/common/tokens.o: frontend/common/tokens.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/common/tree_func.o: frontend/common/tree_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/include/operators_func.o: frontend/include/operators_func.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/debug_output/graphviz_dump.o: frontend/debug_output/graphviz_dump.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/io/put_tree_to_file.o: frontend/io/put_tree_to_file.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/io/read_program.o: frontend/io/read_program.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/syntax_parse/make_tokens_tree.o: frontend/syntax_parse/make_tokens_tree.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

frontend/tokenizing/tokenize.o: frontend/tokenizing/tokenize.cpp
	$(COMP) -c $< -o $@ $(CFLAGS)

lang: frontend/main_frontend.o frontend/common/metki.o frontend/common/tokens.o frontend/common/tree_func.o frontend/debug_output/graphviz_dump.o frontend/io/put_tree_to_file.o frontend/io/read_program.o frontend/syntax_parse/make_tokens_tree.o frontend/tokenizing/tokenize.o frontend/include/operators_func.o
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check: lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./lang

run_debug: lang
	ASAN_OPTIONS="help=1:verbosity=3:detect_stack_use_after_return=1:check_initialization_order=1:detect_container_overflow=1:strict_string_checks=1:detect_invalid_pointer_pairs=2:halt_on_error=0" ./lang

run_with_gdb: lang
	ASAN_OPTIONS="verbosity=2:abort_on_error=0" gdb --args ./lang

# $@ имя цели
# $^ все зависимости
# $(COMP) clang++
# clang++ -o stack main.o hash.o log.o my_assert.o stack_func.o

clean:
	rm -f lang  frontend/*.o frontend/common/*.o frontend/debug_output/*.o frontend/io/*.o frontend/syntax_parse/*.o frontend/tokenizing/*.o frontend/debug_output/images/*.dot frontend/debug_output/images/*.svg frontend/include/*.o