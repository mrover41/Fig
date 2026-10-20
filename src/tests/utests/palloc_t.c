#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <utest/palloc_t.h>
#include <api/ppayload.h>
#include <api/process.h>

bool palloc_t() {
	pid_t child = fork();
	if (child < 0) return false;
	if (child == 0) {
		while (1) pause();
		exit(0);
	}

	process_data *data = pattach(child);
	if (data == NULL) {
		puts("\t [PALLOC_T] ERROR IN TEST: pattach() filed\n");
		return false;
	}

	bool res = palloc(data);

	kill(child, SIGKILL);
	waitpid(child, NULL, 0);
	return res;
}
