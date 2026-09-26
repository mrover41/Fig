#include <stdio.h>
#include <stdlib.h>

#include "process.h"

static process_data *data;

int main(int argc, char *argv[]) { //TODO: Write loader
	if (argc < 2) {
		puts("use ./this_programm pid");
		return 0;
	}

	int pid = atoi(argv[1]);

	/*
	ssize_t id;
	if ((id = pattach(pid, data)) < 0) return -1;
	// Load libs and other code
	pdetach((size_t)id);*/

	return 0;
}
