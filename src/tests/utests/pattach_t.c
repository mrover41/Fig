#include <sys/types.h>
#include <stddef.h>

#include <utest/pattach_t.h>
#include <utest/mflag.h>
#include <types/pdata.h>
#include <api/process.h>

bool pattach_t(pid_t pid, process_data **data) {
	*data = pattach(pid);
	return data != NULL;
}

bool pattach_malloc_t(pid_t pid, process_data **data) {
	_tmalloc(true);
	*data = pattach(pid);
	_tmalloc(false);

	return data == NULL;
}
