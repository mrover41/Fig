#include <sys/ptrace.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <types/pdata.h>
#include <api/process.h>
#include <api/procinf.h>
#include <utest/mflag.h> //example

#include "processinf.h"

process_info *data_arr = NULL;
size_t data_arr_size = 0;

ssize_t getIndex(process_data *data);

process_data *pattach(pid_t pid) {
	long r;
	if ((r = ptrace(PTRACE_ATTACH, pid, NULL, NULL)) < 0) {
		fprintf(stderr, "Ptrace error, error code %li\n", r);
		return NULL;
	}

	if (data_arr == NULL) {
		_tmalloc(true); //example
		if ((data_arr = malloc(sizeof(process_info))) == NULL) {
			_tmalloc(false); //example
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return NULL;
		}
		data_arr_size++;
	} else {
		process_info *arr_cpy = data_arr;
		if ((arr_cpy = realloc(arr_cpy, sizeof(process_info) * (data_arr_size + 1))) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return NULL;
		};

		data_arr = arr_cpy;
		data_arr_size++;
	}

	char name[BUFFER_SIZE];
	getName(pid, name);

	size_t bAddr = 0;
	size_t bAddre = 0;
	if (!getBaseAddr(pid, name, &bAddr, &bAddre)) {
		fprintf(stderr, "Get base adress error, pid: %i, name: %s, base: %li, base end: %li\n", pid, name, bAddr, bAddre);
		return NULL;
	};

	data_arr[data_arr_size - 1] = (process_info) {
		.pdata = (process_data) {
			.pid = pid,
			.base = (void *)bAddr,
			.base_end = (void *)bAddre,
		},
		.allocAddr = NULL,
	};
	memcpy(&data_arr[data_arr_size - 1].name, name, sizeof(name));

	return &data_arr[data_arr_size - 1].pdata;
}

void pdetach(process_data *data) {
	if (data == NULL) return;

	ssize_t index;
	if ((index = getIndex(data)) < 0) {
		fputs("get index error\n", stderr);
		return;
	}

	for (size_t current = index; current < data_arr_size - 1; current++) { //-1 something we could use current + 1
		data_arr[current] = data_arr[current + 1];
	}

	data_arr_size--;

	if (data_arr_size <= 0) { //ya, realloc(.., 0) = free(...) but i`m make a null check below
		free(data_arr);
		data_arr = NULL;
		return;
	}

	process_info *arr_cpy = data_arr;
	if ((arr_cpy = realloc(arr_cpy, sizeof(process_info) * data_arr_size)) == NULL) {
		fputs("error in realloc, memory leak\n", stderr);
		return;
	};

	data_arr = arr_cpy;
	return;
}

void pdetach_all() {
	for (size_t cur = 0; cur < data_arr_size; cur++) {
		pplay(&data_arr[cur].pdata);
		ptrace(PTRACE_DETACH, data_arr[cur].pdata.pid, NULL, NULL);
	}

	data_arr_size = 0;
	free(data_arr);
}

bool ppause(process_data *data) {
	if (data == NULL) return false;

	ptrace(PTRACE_INTERRUPT, data->pid, NULL, NULL);

	return true;
}

bool pplay(process_data *data) {
	if (data == NULL) return false;
	ptrace(PTRACE_CONT, data->pid, NULL, NULL);

	return true;
}

process_info *get_pparent(process_data *data) {
	return (process_info*)((char*)data - offsetof(process_info, pdata));
}

ssize_t getIndex(process_data *data) {
	process_info *parent = get_pparent(data);
	size_t index = (parent - data_arr);

	if (index >= data_arr_size) {
		fputs("out of range, memory leak\n", stderr);
		return -1;
	}

	return index;
}
