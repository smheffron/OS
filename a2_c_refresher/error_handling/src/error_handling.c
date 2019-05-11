#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "../include/error_handling.h"

int create_blank_records(Record_t **records, const size_t num_records)
{
	*records = (Record_t*) malloc(sizeof(Record_t) * num_records);
	memset(*records,0,sizeof(Record_t) * num_records);
	return 0;	
}

int read_records(const char *input_filename, Record_t *records, const size_t num_records) {

	int fd = open(input_filename, O_RDONLY);

	ssize_t data_read = 0;
	for (size_t i = 0; i < num_records; ++i) {
		data_read = read(fd,&records[i], sizeof(Record_t));	
	}
	return 0;
}

int create_record(Record_t **new_record, const char* name, int age)
{
	*new_record = (Record_t*) malloc(sizeof(Record_t));
	
	memcpy((*new_record)->name,name,sizeof(char) * strlen(name));
	(*new_record)->name[MAX_NAME_LEN - 1] = 0;	
	(*new_record)->age = age;
	return 0;

}
