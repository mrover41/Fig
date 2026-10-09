#ifndef MFLAG_H
#define MFLAG_H

#include <sys/types.h>

void _tmalloc(const bool *, size_t);
void _tmalloc_disable();
bool _gmalloc();

#endif
