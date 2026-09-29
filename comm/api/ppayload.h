#ifndef PAYLOD_H
#define PAYLOAD_H

#include <types/pdata.h>
#include <sys/types>

bool palloc(process_data *);
bool pexecute(void (*)(), process_data *);

#endif
