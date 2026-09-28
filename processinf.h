#ifndef PROCESSI_H
#define PROCESSI_H

typedef struct {
	process_data pdata;
	char name[BUFFER_SIZE];
	void *allocAddr;
} process_info;

process_info *get_pparent(process_data *data);

#endif
