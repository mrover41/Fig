#include <sys/ptrace.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <alloca.h>
#include <stdio.h>
#include <math.h>

#include <types/pdata.h>
#include <api/process.h>

#include "processinf.h"

process_info *data_arr = NULL;
size_t data_arr_size = 0;

void *getBaseAddr(pid_t, const char *);
void getName(pid_t, void *);

ssize_t getIndex(process_data *data);

process_data *pattach(pid_t pid) {
	long r;
	if ((r = ptrace(PTRACE_ATTACH, pid, NULL, NULL)) < 0) {
		fprintf(stderr, "Ptrace error, error code %li\n", r);
		return NULL;
	}

	if (data_arr == NULL) {
		if ((data_arr = malloc(sizeof(process_info))) == NULL) {
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

	data_arr[data_arr_size - 1] = (process_info) {
		.pdata = (process_data) {
			.pid = pid,
			.base = getBaseAddr(pid, name),
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


void *getBaseAddr(pid_t pid, const char *name) {
	char *maps_path;

	size_t length = (pid > 0 ? log10(pid) + 1 : 1);
	maps_path = alloca(length + 12);

	sprintf(maps_path, "/proc/%i/maps", pid);
#ifdef DEBUG
	fprintf(stdout, "maps path: %s\n", maps_path);
	fprintf(stdout, "maps str length: %li + 12 = %li\n", length, length + 12);
#endif

	FILE *fp = fopen(maps_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file error: %s\n", maps_path);
		return NULL;
	}

	char *line = NULL;
	size_t len = 0;
	while(getdelim(&line, &len, '\n', fp) != -1) {
		char *n;
		if((n = strrchr(line, '/')) == NULL) continue;
		n += 1;
		if (strcmp(n, name) == 0) break;
#ifdef DEBUG
		fprintf(stdout, "Str not have '/': line: %s, n: %s\n", line, n);
#endif
	}
	
	char *addrs = NULL;
	if ((addrs = strchr(line, '-')) == NULL) {
		free(line);
		fclose(fp);
		return NULL;
	}

	*addrs = '\0';
	size_t res = strtoul(line, NULL, 16);

#ifdef DEBUG
	fprintf(stdout, "Base addr found, base adress: \n%li\nstring:\n%s\n", res, line);
#endif

	free(line);
	fclose(fp);
	return (void *)res;
}

void getName(pid_t pid, void *buff) {
	sprintf(buff, "/proc/%d/comm", pid);

	FILE *fp = fopen(buff, "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file error: %s\n", (char *)buff);
		return;
	}
	
	if (fgets(buff, BUFFER_SIZE, fp) != NULL) {
#ifdef DEBUG
		fprintf(stdout, "[PROCESS_C] Programm name readed: %s\n", (char *)buff);
#endif
		fclose(fp);
		return;
	}

	fprintf(stderr, "Read file error: %s\n", (char *)buff);
	fclose(fp);
	buff = NULL;
	return;
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
