.PHONY: all clean run_leak_check_front run_leak_check_back run_leak_check_lang frontend_lang backend_lang
# эти цели не являются файлами выполняй их даже если соотв файлы существуют
all: lang
# когда запускаем make без цели, то выполняем первую цель после all, то есть записи make stack make all и make эквивалентны

COMP=clang++

# FLAGS ------------------------------------------------------------------

CFLAGS_DEBUG = -D_DEBUG -DDEBUG_SMALL_TREE
COMMON_CFLAGS =  -DASSEMBLER -DPROCESSOR -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions -g -fsanitize=address,leak,undefined    
LDFLAGS = -fsanitize=address,leak,undefined
CFLAGS_ASM = -DASSEMBLER -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions  -g -fsanitize=address,leak,undefined 
CFLAGS_PROC = -DPROCESSOR -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192 -fPIE -Werror=vla -Wno-c++11-extensions  -g -fsanitize=address,leak,undefined 

ifdef DEBUG_TREE
    CFLAGS = $(COMMON_CFLAGS) $(CFLAGS_DEBUG)
else
    CFLAGS = $(COMMON_CFLAGS)
endif

# СOMMON ------------------------------------------------------------------

STK = $(shell find stack -type f -name "*.cpp")
STK_OBJS = $(STK:%.cpp=%.o)

TREE = $(shell find tree -type f -name "*.cpp")
TREE_OBJS = $(TREE:%.cpp=%.o)

$(STK_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(TREE_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

# FRONTEND ------------------------------------------------------------------

FRNTD = $(shell find frontend -type f -name "*.cpp")
FRNTD_OBJS = $(FRNTD:%.cpp=%.o)

$(FRNTD_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

frontend_lang: $(FRNTD_OBJS) $(STK_OBJS) $(TREE_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_front: frontend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./frontend_lang


# BACKEND ------------------------------------------------------------------

BCKND_NOT_PROC_ASM = $(shell find backend -type f -name "*.cpp" -not -path "*/Processor-and-assembler/*")
BCKND_NOT_PROC_OBJS = $(BCKND_NOT_PROC_ASM:%.cpp=%.o)

BCKND_ASM = $(shell find backend/Processor-and-assembler -type f -name "*.cpp" -not -path  "*/processor_task*")
BCKND_ASM_OBJS = $(BCKND_ASM:%.cpp=%.o)

BCKND_PROC = $(shell find backend/Processor-and-assembler -type f -name "*.cpp" -not -path "*/assembler_task*")
BCKND_PROC_OBJS = $(BCKND_PROC:%.cpp=%.o)

$(BCKND_NOT_PROC_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS) 

$(BCKND_ASM_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS_ASM)

$(BCKND_PROC_OBJS): %.o: %.cpp                            
	$(COMP) -c $< -o $@ $(CFLAGS_PROC)


backend_lang: $(STK_OBJS) $(TREE_OBJS) $(BCKND_NOT_PROC_OBJS) $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_back: backend_lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./backend_lang

# Frontend and backend --------------------------------------------------------

lang: $(FRNTD_OBJS) $(STK_OBJS) $(TREE_OBJS) $(BCKND_NOT_PROC_OBJS) $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS)
	$(COMP) -o $@ $^ $(LDFLAGS)

run_leak_check_lang: lang
	ASAN_OPTIONS="detect_leaks=1:verbosity=1:print_stacktrace=1" ./lang

clean:
	rm -f frontend_lang backend_lang lang
	rm -f $(FRNTD_OBJS) $(STK_OBJS) $(TREE_OBJS) $(BCKND_NOT_PROC_OBJS) $(BCKND_ASM_OBJS) $(BCKND_PROC_OBJS)
	rm -f frontend/debug_output_frontend/images/*.dot
	rm -f frontend/debug_output_frontend/images/*.svg
	rm -f backend/make_backend_tree/debug_output_backend/images/*.dot
	rm -f backend/make_backend_tree/debug_output_backend/images/*.svg