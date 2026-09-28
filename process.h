#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define BUFFER_SIZE 255

typedef struct {
	pid_t pid;
	const char *name;
	const void *base;
} process_data;

process_data *pattach(pid_t);
void pdetach(process_data *);
void pdetach_all();

bool ppause(process_data *);
bool pplay(process_data *);
bool psteap(process_data *);

bool pallocate_mem(void *, process_data *);
bool pexecute(void (*)(), process_data *);

#endif
