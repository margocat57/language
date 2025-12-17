#ifndef OPERATOR_FUNC_H
#define OPERATOR_FUNC_H
#include <string.h>
#include "../tree/tree.h"
#include <stdio.h>
#include "../backend/make_backend_tree/find_rbp_shift/calcul_rbp/calcul_rbp.h"


#ifdef CALCUL_SHIFT
    #define SHIFT_RBP(...) __VA_ARGS__
#else 
    #define SHIFT_RBP(...)
#endif //PROCESSOR

struct op_info{
    // reading from file
    OPERATORS op;
    const char* op_name_in_code;
    size_t num_of_symb_code;
    const char* op_name_dump;
    // dump
    const char* name_for_graph_dump;
    // shift count
    SHIFT_RBP(void(*shift_func)(TreeNode_t*, Stack_t*, name_table*,  size_t*,  TreeErr_t*, bool));
};


const op_info OPERATORS_INFO[] = {
    {INCORR},
    {OP_ADD,         "cretino",          sizeof("cretino")         - 1,  "+",      "OP_ADD",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_SUB,         "pazzi",            sizeof("pazzi")           - 1,  "-",      "OP_SUB",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_MUL,         "merda",            sizeof("merda")           - 1,  "*",      "OP_MUL",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_DIV,         "scemo",            sizeof("scemo")           - 1,  "/",      "OP_DIV",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_DEG,         "mona",             sizeof("mona")            - 1,  "^",      "OP_DEG",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_INIT,        "fare_pompino",     sizeof("fare_pompino")    - 1,  ":=",     "OP_INIT",  SHIFT_RBP(CalculRpbShiftInit)},
    {OP_ASS,         "a_fanculo",        sizeof("a_fanculo")       - 1,  "=",      "OP_ASS",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_EQ,          "сazzo,fa",         sizeof("сazzo,fa")        - 1,  "==",     "OP_EQ",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_GE,          "сazzo_lungo",      sizeof("сazzo_lungo")     - 1,  ">",      "OP_GE",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_LE,          "cazzo_piu_corto",  sizeof("cazzo_piu_corto") - 1,  "<",      "OP_LE",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_SP,          "porco_dio!",       sizeof("porco_dio!")      - 1,  ";",      "OP_SP",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_IF,          "Culo",             sizeof("Culo")            - 1,  "if",     "OP_IF",    SHIFT_RBP(CalculRpbShiftIfElseWhile)},
    {OP_ELSE,        "Fica",             sizeof("Fica")            - 1,  "else",   "OP_ELSE",  SHIFT_RBP(CalculRpbShiftIfElseWhile)},
    {OP_WHILE,       "Fottere",          sizeof("Fottere")         - 1,  "while",  "OP_WHILE", SHIFT_RBP(CalculRpbShiftIfElseWhile)},
    {OP_OPEN_BR,     "(Ciucciami",       sizeof("(Ciucciami")      - 1,  "(",      "OP_OB",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_CLOSE_BR,    "il_cazzo!)",       sizeof("il_cazzo!)")      - 1,  ")",      "OP_CB",    SHIFT_RBP(CalculRpbShiftOther)},
    {OP_OPEN_FIG_BR, "{Voi-Figlio",      sizeof("{Voi-Figlio")     - 1,  "{",      "OP_OFB",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_CLOSE_FIG_BR, "di_putana}",      sizeof("di_putana}")      - 1,  "}",      "OP_CFB",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_COMMA,        "сhe_culo",        sizeof("сhe_culo")        - 1,  ",",      "OP_COM",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_RETURN,       "succhia",         sizeof("succhia")         - 1,  "return", "OP_RET",   SHIFT_RBP(CalculRpbShiftOther)},
    {OP_INPUT,        "Infila_сazzo",    sizeof("Infila_сazzo")    - 1,  "input",  "OP_INPUT", SHIFT_RBP(CalculRpbShiftOther)},
    {OP_OUTPUT,       "Estrarre_cazzo",  sizeof("Estrarre_cazzo")  - 1,  "output", "OP_OUTPUT",SHIFT_RBP(CalculRpbShiftOther)},
    {OP_EXIT,         "Basta!",          sizeof("Basta!")          - 1,  "exit",   "OP_EXIT",  SHIFT_RBP(CalculRpbShiftOther)},
};

typedef uint64_t Op_info_mistake_t;

Op_info_mistake_t find_operators_info_mistake();

enum OPERATORS_INFO_MISTAKES{
    NO_MISTAKE_OP,
    INDEX_AND_OP_NOT_SIMMILAR,
    INCORR_LEN
};



#endif //OPERATOR_FUNC_H