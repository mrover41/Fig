#ifndef PDATA_H
#define PDATA_H

typedef struct {
	pid_t pid;
	const void *base;
} process_data;

#endif
