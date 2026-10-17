#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

#include <utest/pattach_t.h>
#include <utest/mflag.h>

#include <types/pdata.h>
#include <api/process.h>

static const bool mask[] = {true};

bool pattach_t() {
	pid_t child = fork();
	if (child < 0) return false;
	if (child == 0) {
		while (1) pause();
		exit(0);
	}

	sleep(1);

	process_data *data = pattach(child);

	kill(child, SIGKILL);
	waitpid(child, NULL, 0);

	pdetach(data);
	return data != NULL;
}

bool pattach_malloc_t(pid_t pid) {
	_tmalloc(mask, sizeof(mask) / sizeof(mask[0]));
	process_data *data = pattach(pid);
	_tmalloc_disable();

	return data == NULL;
}
