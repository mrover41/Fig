#include <sys/types.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/user.h>
#include <stddef.h>

#include "process.h"

typedef struct {
	process_data pdata;
	void *allocAddr;
} process_info;

process_info *data_arr = NULL;
size_t data_arr_size = 0;

//void *getBaseAddr(pid_t, const char *name);
char *getName(pid_t);

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
		if ((arr_cpy = realloc(arr_cpy, sizeof(process_info) * data_arr_size + 1)) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return NULL;
		};

		data_arr = arr_cpy;
		data_arr_size++;
	}

	char *name = getName(pid);
	data_arr[data_arr_size - 1] = (process_data) {
		.process_pid = pid,
		//.base = getBaseAddr(pid, name) TODO: get base adress
		.name = name,
	};

	data_arr[data_arr_size - 1] = (process_info) {
		.pdata = (process_data) {
			.pid = pid,
			//.base = getBaseAddr(pid, name), TODO: get base adress
			.name = name,
		},
		.allocAddr = NULL,
	};

	return &data_arr[data_arr_size - 1].pdata;
}

void pdetach(process_data *data) {
	if (data == NULL) return;

	size_t index;
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

bool pallocate_mem(void *addr, process_data *data) {
	size_t index;
	if ((index = getIndex(data)) < 0) {
		fputs("get index error\n", stderr);
		return false;
	}

	ppause(data);

	struct user_regs_struct old_regs, regs;
	ptrace(PTRACE_GETREGS, data->pid, NULL, &regs);

	old_regs = regs;

	regs.rax = 0x09;
	regs.rdi = 0;
	regs.rsi = 4096;
	regs.rdx = 0x1 | 0x2 | 0x4;
	regs.r10 = 0x02 | 0x20;
	regs.r8  = (unsigned long long)-1;
	regs.r9  = 0;

	regs.orig_rax = -1; //Kernel can save RAX before interrupt syscall, we must reset this

	ptrace(PTRACE_SETREGS, data->pid, NULL, &regs);
	ptrace(PTRACE_SINGLESTEP, data->pid, NULL, NULL);

	ptrace(PTRACE_GETREGS, data->pid, NULL, &regs);
	data_arr[index].allocAddr = (void *)regs.rax;

	ptrace(PTRACE_SETREGS, data->pid, NULL, &old_regs);

	return true;
}

bool pexecute(void (*method)(), process_data *data) {
	size_t index;
	if ((index = getIndex(data)) < 0) {
		fputs("get index error\n", stderr);
		return false;
	}

	if (data_arr[index].allocAddr == NULL) return false;
	
	//TODO: Inject code

	return true;
}

void *getBaseAddr(pid_t pid, const char *name) {
	//string maps_path = "/proc/" + to_string(pid) + "/maps";
}

char *getName(pid_t pid) {
	char buff[BUFFER_SIZE];

	sprintf(buff, "/proc/%d/comm", pid);

	FILE *fp = fopen(buff, "r");
	if (fp == NULL) {
		fprintf(stderr, "Open file error: %s\n", buff);
		return NULL;
	}
	
	if (fgets(buff, BUFFER_SIZE, fp) != NULL) {
#ifdef DEBUG
		fprintf(stdout, "[PROCESS_C] Programm name readed: %s\n", buff);
#endif
		fclose(fp);
		return buff;
	}

	fprintf(stderr, "Read file error: %s\n", buff);
	fclose(fp);
	return NULL;
}

ssize_t getIndex(process_data *data) {
	process_info *parent = (process_info*)((char*)data - offsetof(process_info, pdata));
	size_t index = (parent - data_arr);

	if (index >= data_arr_size) {
		fputs("out of range, memory leak\n", stderr);
		return -1;
	}

	return index;
}
