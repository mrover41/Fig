#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

typedef struct {
	pid_t process_pid;
	void *base;
} process_data;

ssize_t pattach(pid_t, process_data *);
void pdetach(size_t);

#endif
