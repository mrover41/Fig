#ifndef PALLOCT_H
#define PALLOCT_H

#include <types/pdata.h>
#include <sys/types.h>

bool pattach_t(pid_t, process_data **);
bool pattach_malloc_t(pid_t, process_data **);

#endif
