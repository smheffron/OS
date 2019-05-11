#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/arrays.h"

// LOOK INTO MEMCPY, MEMCMP, FREAD, and FWRITE

bool array_copy(const void *src, void *dst, const size_t elem_size, const size_t elem_count)
{

}

bool array_is_equal(const void *data_one, void *data_two, const size_t elem_size, const size_t elem_count)
{

}

ssize_t array_locate(const void *data, const void *target, const size_t elem_size, const size_t elem_count)
{

}

bool array_serialize(const void *src_data, const char *dst_file, const size_t elem_size, const size_t elem_count)
{

}

bool array_deserialize(const char *src_file, void *dst_data, const size_t elem_size, const size_t elem_count)
{

}

