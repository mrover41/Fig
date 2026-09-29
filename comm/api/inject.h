#ifndef INJECT_H
#define INJECT_H

#include <types/pdata.h>

bool pallocate_mem(void *, process_data *);
bool pexecute(void (*)(), process_data *);

#endif
