#ifndef PDATA_H
#define PDATA_H

typedef struct {
	pid_t pid;
	const void *base;
	const void *base_end;
} process_data;

#endif
