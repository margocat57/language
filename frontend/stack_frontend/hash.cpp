#include <stdlib.h>
#include <stdio.h>
#include "hash.h"
#include "my_assert.h"
#include "stack_func.h"

size_t create_djb2_hash(char *data, size_t size)
{
    if (!data)
        return 1;

    size_t hash = 5381;

    for (size_t i = 0; i < size; i++)
    {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }

    return hash;
}
