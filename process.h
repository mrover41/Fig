#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define BUFFER_SIZE 255

typedef struct {
	pid_t pid;
	const void *base;
} process_data;

typedef struct {
	process_data pdata;
	char name[BUFFER_SIZE];
	void *allocAddr;
} process_info;

process_data *pattach(pid_t);
void pdetach(process_data *);
void pdetach_all();

bool ppause(process_data *);
bool pplay(process_data *);
bool psteap(process_data *);

bool pallocate_mem(void *, process_data *);
bool pexecute(void (*)(), process_data *);

process_info *get_pparrent(process_data *data);

#endif
