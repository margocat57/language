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

//! Верификатор табличек(всех)
//! под макросом


const op_info OPERATORS_INFO[] = {
    {INCORR},
    {OP_ADD,         "cretino",          sizeof("cretino")         - 1,  "+",      "OP_ADD"},
    {OP_SUB,         "pazzi",            sizeof("pazzi")           - 1,  "-",      "OP_SUB"},
    {OP_MUL,         "merda",            sizeof("merda")           - 1,  "*",      "OP_MUL"},
    {OP_DIV,         "scemo",            sizeof("scemo")           - 1,  "/",      "OP_DIV"},
    {OP_DEG,         "mona",             sizeof("mona")            - 1,  "^",      "OP_DEG"},
    {OP_INIT,        "fare_pompino",     sizeof("fare_pompino")    - 1,  ":=",     "OP_INIT"},
    {OP_ASS,         "a_fanculo",        sizeof("a_fanculo")       - 1,  "=",      "OP_ASS"},
    {OP_EQ,          "сazzo,fa",         sizeof("сazzo,fa")        - 1,  "==",     "OP_EQ"},
    {OP_GE,          "сazzo_lungo",      sizeof("сazzo_lungo")     - 1,  ">",      "OP_GE"},
    {OP_LE,          "cazzo_piu_corto",  sizeof("cazzo_piu_corto") - 1,  "<",      "OP_LE"},
    {OP_SP,          "porco_dio!",       sizeof("porco_dio!")      - 1,  ";",      "OP_SP"},
    {OP_IF,          "Culo",             sizeof("Culo")            - 1,  "if",     "OP_IF"},
    {OP_ELSE,        "Fica",             sizeof("Fica")            - 1,  "else",   "OP_ELSE"},
    {OP_WHILE,       "Fottere",          sizeof("Fottere")         - 1,  "while",  "OP_WHILE"},
    {OP_OPEN_BR,     "(Ciucciami",       sizeof("(Ciucciami")      - 1,  "(",      "OP_OB"},
    {OP_CLOSE_BR,    "il_cazzo!)",       sizeof("il_cazzo!)")      - 1,  ")",      "OP_CB"},
    {OP_OPEN_FIG_BR, "{Voi-Figlio",      sizeof("{Voi-Figlio")     - 1,  "{",      "OP_OFB"},
    {OP_CLOSE_FIG_BR, "di_putana}",      sizeof("di_putana}")      - 1,  "}",      "OP_CFB"},
    {OP_COMMA,        "сhe_culo",        sizeof("сhe_culo")        - 1,  ",",      "OP_COM"},
    {OP_RETURN,       "succhia",         sizeof("succhia")         - 1,  "return", "OP_RET"},
    {OP_AND,          "porco_cane",      sizeof("porco_cane")      - 1,  "&&",     "OP_AND"},
    {OP_OR,           "porca_troia",     sizeof("porca_troia")     - 1,  "||",     "OP_OR"},
};


#endif //OPERATOR_FUNC_H