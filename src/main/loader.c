#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <api/ppayload.h>
#include <api/process.h>

static process_data *data;

int main(int argc, char *argv[]) { //TODO: Write loader
	if (argc < 2) {
		puts("use ./this_programm pid");
		return 0;
	}

	pid_t pid = 0;

	puts("[LOADER] Loading...");

	if ((pid = atoi(argv[1])) == 0) return -1;

#ifdef DEBUG
	printf("[LOADER] PID: %i, try to pattach()\n", pid);
#endif

	if ((data = pattach(pid)) == NULL) return -1;
#ifdef DEBUG
	puts("[LOADER] Try to palloc()\n");
#endif
	if (!palloc(data))  {
		pdetach_all();
		return -1;
	}

	
	//TODO: Load libs	

#ifdef DEBUG
	puts("[LOADER] Try to pdetach()\n");
#endif
	pdetach_all();
	return 0;
}
