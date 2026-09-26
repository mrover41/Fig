#include <sys/types.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>

#include "process.h"

process_data *data_arr = NULL;
size_t data_arr_size = 0;

//void *getBaseAddr(pid_t);

ssize_t pattach(pid_t pid, process_data *data) {
	long r;
	if ((r = ptrace(PTRACE_ATTACH, pid, NULL, NULL)) < 0) {
		fprintf(stderr, "Ptrace error, error code %li\n", r);
		return -1;
	}

	if (data_arr == NULL) {
		if ((data_arr = malloc(sizeof(process_data))) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return -1;
		}
		data_arr_size++;
	} else {
		process_data *arr_cpy = data_arr;
		if ((arr_cpy = realloc(arr_cpy, sizeof(process_data) * data_arr_size + 1)) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return -1;
		};

		data_arr = arr_cpy;
		data_arr_size++;
	}

	data_arr[data_arr_size - 1] = (process_data) {
		.process_pid = pid,
		//.base = getBaseAddr(pid); TODO: get base adress
	};

	data = data_arr + (data_arr_size - 1);

	return data_arr_size - 1;
}

void pdetach(size_t id) {
#ifdef DEBUG
	if (id >= data_arr_size) {
		fputs("[PROCESS_H] out of range error\n", stderr);
		return;
	}
#endif

	for (size_t current = id; current < data_arr_size - 1; current++) { //-1 something we could use current + 1
		data_arr[current] = data_arr[current + 1];
	}

	data_arr_size--;

	if (data_arr_size <= 0) { //ya, realloc(.., 0) = free(...) but i`m make a null check below
		free(data_arr);
		data_arr = NULL;
		return;
	}

	process_data *arr_cpy = data_arr;
	if ((arr_cpy = realloc(arr_cpy, sizeof(process_data) * data_arr_size)) == NULL) {
		fputs("error in realloc, memory leak\n", stderr);
		return;
	};

	data_arr = arr_cpy;
	return;
}

//	free(data_arr);


void *getBaseAddr(pid_t pid) {
	//string maps_path = "/proc/" + to_string(pid) + "/maps";
}
