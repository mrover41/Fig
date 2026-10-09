#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//#include <api/ppayload.h>
#include <utest/pattach_t.h>
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

	puts("\t[-] TESTS BEGIN [-]\n");

	if (pattach_t(pid, &data)) puts("\tTEST: Pattach_t done\n");
	if (pattach_malloc_t(pid, &data)) puts("\tTEST: Pattach_malloc_t done\n"); //TODO: fix logging in this test

	puts("\t[-] TESTS END [-]\n");

	//TODO: Load libs	

#ifdef DEBUG
	puts("[LOADER] Try to pdetach()\n");
#endif
	pdetach_all();
	return 0;
}
