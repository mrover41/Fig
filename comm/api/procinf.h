#ifndef PINF_H
#define PINF_H

#include <sys/types.h>

void getName(pid_t, void *);
bool getBaseAddr(const pid_t, const char *, size_t *, size_t *);

#endif
