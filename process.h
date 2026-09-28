#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define BUFFER_SIZE 255

typedef struct {
	pid_t pid;
	const char *name;
	const void *base;
} process_data;

ssize_t pattach(pid_t, process_data *);
void pdetach(size_t);
void pdetach_all();

bool ppause(size_t);
bool pplay(size_t);
bool psteap(size_t);

bool pallocate_mem(void *, size_t);
bool pexecute(void (*)(), size_t);

#endif
