#ifndef PROCESSI_H
#define PROCESSI_H

#include <types/pdata.h>

#define BUFFER_SIZE 255

typedef struct {
	process_data pdata;
	void *allocAddr;
	char name[BUFFER_SIZE];
} process_info;

process_info *get_pparent(process_data *data);

#endif
