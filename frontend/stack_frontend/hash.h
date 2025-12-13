//!@file
#ifndef HASH_H
#define HASH_H
#include "stdlib.h"
#include "stack_func.h"

//! Calculates hash for data using DJB2 algorithm
//! Starts with initial number 5381
//! For each byte of data: hash = (hash * 33) + current_byte
size_t create_djb2_hash(char* data, size_t size);

#endif //HASH_H