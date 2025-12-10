#ifndef OPERATOR_FUNC_H
#define OPERATOR_FUNC_H
#include <string.h>
#include "tree.h"
#include <stdio.h>


struct op_info{
    // reading from file
    OPERATORS op;
    const char* op_name_in_code;
    size_t num_of_symb_code;
    const char* op_name_dump;
    // dump
    const char* name_for_graph_dump;
};

// Cazzo_lungo
// Cazzo_piu_corto


const op_info OPERATORS_INFO[] = {
    {INCORR},
    {OP_ADD,         "cretino",         sizeof("cretino"),         "+",     "OP_ADD"},
    {OP_SUB,         "pazzi",           sizeof("pazzi"),           "-",     "OP_SUB"},
    {OP_MUL,         "merda",           sizeof("merda"),           "*",     "OP_MUL"},
    {OP_DIV,         "scemo",           sizeof("scemo"),           "/",     "OP_DIV"},
    {OP_DEG,         "mona",            sizeof("mona"),            "^",     "OP_DEG"},
    {OP_INIT,        "fare_pompino",    sizeof("fare_pompino"),    ":=",    "OP_INIT"},
    {OP_ASS,         "a_fanculo",       sizeof("a_fanculo"),       "=",     "OP_ASS"},
    {OP_EQ,          "сazzo,fa",        sizeof("сazzo,fa"),        "==",    "OP_EQ"},
    {OP_GE,          "сazzo_lungo",     sizeof("сazzo_lungo"),     ">",     "OP_GE"},
    {OP_LE,          "cazzo_piu_corto", sizeof("cazzo_piu_corto"), "<",     "OP_LE"},
    {OP_SP,          "porco_dio!",      sizeof("porco_dio!"),      ";",     "OP_SP"},
    {OP_IF,          "Culo",            sizeof("Culo"),            "if",    "OP_IF"},
    {OP_ELSE,        "Fica",            sizeof("Fica"),            "else",  "OP_ELSE"},
    {OP_WHILE,       "Fottere",         sizeof("Fottere"),         "while", "OP_WHILE"},
    {OP_OPEN_BR,     "(Ciucciami",      sizeof("(Ciucciami"),      "(",     "OP_OB"},
    {OP_CLOSE_BR,    "il_cazzo!)",      sizeof("il_cazzo!)"),      ")",     "OP_CB"},
    {OP_OPEN_FIG_BR, "{Voi-Figlio",     sizeof("{Voi-Figlio"),     "{",     "OP_OFB"},
    {OP_CLOSE_FIG_BR, "di_putana}",     sizeof("di_putana}"),      "}",     "OP_CFB"},
};  

#endif //OPERATOR_FUNC_H