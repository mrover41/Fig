#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define BUFFER_SIZE 255

typedef struct {
	pid_t process_pid;
	const char *name;
	const void *base;
} process_data;

ssize_t pattach(pid_t, process_data *);
void pdetach(size_t);

#endif
