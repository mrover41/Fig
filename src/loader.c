#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <api/process.h>

static process_data *data;

int main(int argc, char *argv[]) { //TODO: Write loader
	if (argc < 2) {
		puts("use ./this_programm pid");
		return 0;
	}

	pid_t pid = 0;
	if ((pid = atoi(argv[1])) == 0) return -1;

	if ((data = pattach(pid)) < 0) return -1;
	
	//TODO: Load libs	

	pdetach_all();
	return 0;
}
