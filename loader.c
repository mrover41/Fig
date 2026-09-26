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

	/*int id = pattach(pid, data);
	// Load libs and other code
	pdetach(id);*/

	return 0;
}
