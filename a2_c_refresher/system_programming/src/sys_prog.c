#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/sys_prog.h"

// LOOK INTO OPEN, READ, WRITE, CLOSE, FSTAT/STAT, LSEEK
// GOOGLE FOR ENDIANESS HELP

bool bulk_read(const char *input_filename, void *dst, const size_t offset, const size_t dst_size)
{

}

bool bulk_write(const void *src, const char *output_filename, const size_t offset, const size_t src_size)
{

}


bool file_stat(const char *query_filename, struct stat *metadata)
{

}

bool endianess_converter(uint32_t *src_data, uint32_t *dst_data, const size_t src_count)
{

}

