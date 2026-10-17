#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//#include <api/ppayload.h>

#include <api/process.h>
#include <utest/pattach_t.h>
#include <utest/palloc_t.h>

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

	if (pattach_t()) puts("\t\033[32mTEST: Pattach_t done\033[0m\n");
	else puts("\t\033[31mTEST: Pttach_t filed\033[0m\n");

	if (pattach_malloc_t(pid)) puts("\t\033[32mTEST: Pattach_malloc_t done\033[0m\n");
	else puts("\t\033[31mTEST: Pttach_malloc_t filed\033[0m\n");

	if (palloc_t()) puts("\t\033[32mTEST: Palloc_t done\033[0m\n");
	else puts("\t\033[31mTEST: Palloc_t filed\033[0m\n");

	puts("\t[-] TESTS END [-]\n");

	//TODr: Load libs	

#ifdef DEBUG
	puts("[LOADER] Try to pdetach()\n");
#endif
	pdetach_all();
	return 0;
}
