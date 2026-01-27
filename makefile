.PHONY: all clean run_leak_check_front run_leak_check_back run_leak_check_lang frontend_lang backend_lang middleend_lang run_leak_check_middle generate_asm run_leak_check_gen_asm
# эти цели не являются файлами выполняй их даже если соотв файлы существуют
all: lang run_leak_check_lang
# когда запускаем make без цели, то выполняем первую цель после all, то есть записи make stack make all и make эквивалентны

COMP=clang++

# FLAGS ------------------------------------------------------------------

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

# СOMMON ------------------------------------------------------------------

TREE = $(shell find tree -type f -name "*.cpp")
TREE_OBJS = $(TREE:%.cpp=%.o)

DEBUG_OUTPUT = $(shell find debug_output -type f -name "*.cpp")
DEBUG_OUTPUT_OBJS = $(DEBUG_OUTPUT:%.cpp=%.o)

$(TREE_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(DEBUG_OUTPUT_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

# FRONTEND ------------------------------------------------------------------

FRNTD = $(shell find frontend include -type f -name "*.cpp")
FRNTD_OBJS = $(FRNTD:%.cpp=%.o)

$(FRNTD_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

frontend_lang: $(FRNTD_OBJS)  $(TREE_OBJS) $(DEBUG_OUTPUT_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_front: frontend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./frontend_lang

# MIDDLEEND -----------------------------------------------------------------

MDLND = $(shell find middleend -type f -name "*.cpp")
MDLND_OBJS = $(MDLND:%.cpp=%.o)

BCKND_MID_HELP = $(shell find backend/make_backend_tree/read_tree_from_file -type f -name "*.cpp")
BCKND_MID_HELP_OBJS= $(BCKND_MID_HELP:%.cpp=%.o)

PUT_TREE_CPP = $(wildcard frontend/io/put_tree_to_file.cpp)
PUT_TREE_CPP_OBJS= $(PUT_TREE_CPP:%.cpp=%.o)

$(MDLND_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(BCKND_MID_HELP_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(PUT_TREE_CPP_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

middleend_lang: $(MDLND_OBJS) $(TREE_OBJS) $(BCKND_MID_HELP_OBJS) $(DEBUG_OUTPUT_OBJS) $(PUT_TREE_CPP_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_middle: middleend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./middleend_lang


# BACKEND ------------------------------------------------------------------

MAIN_RUN_ASM_CPP = $(wildcard backend/main_run_asm.cpp)
MAIN_RUN_ASM_OBJS= $(MAIN_RUN_ASM_CPP:%.cpp=%.o)

BCKND_NOT_PROC_ASM = $(shell find backend -type f -name "*.cpp" -not -path "*/Processor-and-assembler/*" | grep -v "main_run_asm.cpp")
BCKND_NOT_PROC_OBJS = $(BCKND_NOT_PROC_ASM:%.cpp=%.o)

BCKND_ASM = $(shell find backend/Processor-and-assembler/assembler_task -type f -name "*.cpp")
BCKND_ASM_OBJS = $(BCKND_ASM:%.cpp=%.o)

BCKND_PROC = $(shell find backend/Processor-and-assembler/processor_task -type f -name "*.cpp")
BCKND_PROC_OBJS = $(BCKND_PROC:%.cpp=%.o)

$(BCKND_NOT_PROC_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(MAIN_RUN_ASM_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(BCKND_ASM_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

$(BCKND_PROC_OBJS): %.o: %.cpp    
	$(COMP) -c $< -o $@ $(CFLAGS_PROC)

generate_asm: $(TREE_OBJS) $(BCKND_NOT_PROC_OBJS)  $(DEBUG_OUTPUT_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

backend_lang: $(TREE_OBJS) $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS) $(DEBUG_OUTPUT_OBJS) $(MAIN_RUN_ASM_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_gen_asm: generate_asm
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./generate_asm

run_leak_check_back: backend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./backend_lang

# Frontend, middleend and backend --------------------------------------------------------

lang:
	make frontend_lang && ./frontend_lang && \
	rm -f $(FRNTD_OBJS)	frontend_lang  && make middleend_lang && ./middleend_lang && \
	rm -f $(MDLND_OBJS)	middleend_lang && make generate_asm && ./generate_asm && \
	rm -f $(BCKND_NOT_PROC_OBJS) generate_asm && make backend_lang && ./backend_lang && \
	rm -f $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS) $(TREE_OBJS)

run_leak_check_lang:
	export ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" && \
	make frontend_lang && ./frontend_lang && \
	rm -f $(FRNTD_OBJS)	frontend_lang  && make middleend_lang && ./middleend_lang && \
	rm -f $(MDLND_OBJS)	middleend_lang && make generate_asm && ./generate_asm && \
	rm -f $(BCKND_NOT_PROC_OBJS) generate_asm && make backend_lang && ./backend_lang && \
	rm -f $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS) $(TREE_OBJS)

clean:
	rm -f frontend_lang backend_lang middleend_lang lang generate_asm
	rm -f $(FRNTD_OBJS) $(TREE_OBJS) $(BCKND_NOT_PROC_OBJS) $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS) $(MDLND_OBJS) $(MAIN_RUN_ASM_OBJS) $(DEBUG_OUTPUT_OBJS)
	rm -f debug_output/images/*.dot
	rm -f debug_output/images/*.svg