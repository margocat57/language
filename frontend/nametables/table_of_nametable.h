#ifndef TABLE_OF_NAMETABLE_H
#define TABLE_OF_NAMETABLE_H
#include "nametable.h"

struct table_of_nametable{
    name_table** nametables;
    size_t first_free;
    size_t num_of_nametables;
};

table_of_nametable* TableOfNameTableInit();

size_t TableAddName(table_of_nametable* table, name_table* nametable);

void TableRealloc(table_of_nametable* table, size_t num_of_elem);

void TableDestroy(table_of_nametable* table);

#endif //TABLE_OF_NAMETABLE_H