#include <sys/types.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/user.h>

#include "process.h"

typedef struct {
	process_data pdata;
	void *allocAddr;
} process_info;

process_info *data_arr = NULL;
size_t data_arr_size = 0;

//void *getBaseAddr(pid_t, const char *name);
char *getName(pid_t);

ssize_t pattach(pid_t pid, process_data *data) {
	long r;
	if ((r = ptrace(PTRACE_ATTACH, pid, NULL, NULL)) < 0) {
		fprintf(stderr, "Ptrace error, error code %li\n", r);
		return -1;
	}

	if (data_arr == NULL) {
		if ((data_arr = malloc(sizeof(process_info))) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return -1;
		}
		data_arr_size++;
	} else {
		process_info *arr_cpy = data_arr;
		if ((arr_cpy = realloc(arr_cpy, sizeof(process_info) * data_arr_size + 1)) == NULL) {
			fputs("Memory allocation error\n", stderr);

			ptrace(PTRACE_DETACH, pid);
			return -1;
		};

		data_arr = arr_cpy;
		data_arr_size++;
	}

	char *name = getName(pid);

	data_arr[data_arr_size - 1] = (process_info) {
		.pdata = (process_data) {
			.pid = pid,
			//.base = getBaseAddr(pid, name), TODO: get base adress
			.name = name,
		},
		.allocAddr = NULL,
	};

	data = &data_arr[data_arr_size - 1].pdata;

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

bool ppause(size_t id) {
	if (id >= data_arr_size) {
		fputs("[PROCESS_H] out of range error\n", stderr);
		return false;
	}
	ptrace(PTRACE_INTERRUPT, data_arr[id].pdata.pid, NULL, NULL);

	return true;
}

bool pplay(size_t id) {
	if (id >= data_arr_size) {
		fputs("[PROCESS_H] out of range error\n", stderr);
		return false;
	}
	ptrace(PTRACE_CONT, data_arr[id].pdata.pid, NULL, NULL);

	return true;
}

bool pallocate_mem(void *addr, size_t id) {
	if (id >= data_arr_size) {
		fputs("[PROCESS_H] out of range error\n", stderr);
		return false;
	}

	ppause(id);

	struct user_regs_struct old_regs, regs;
	ptrace(PTRACE_GETREGS, data_arr[id].pdata.pid, NULL, &regs);

	old_regs = regs;

	regs.rax = 0x09;
	regs.rdi = 0;
	regs.rsi = 4096;
	regs.rdx = 0x1 | 0x2 | 0x4;
	regs.r10 = 0x02 | 0x20;
	regs.r8  = (unsigned long long)-1;
	regs.r9  = 0;

	regs.orig_rax = -1; //Kernel can save RAX before interrupt syscall, we must reset this

	ptrace(PTRACE_SETREGS, data_arr[id].pdata.pid, NULL, &regs);
	ptrace(PTRACE_SINGLESTEP, data_arr[id].pdata.pid, NULL, NULL);

	ptrace(PTRACE_GETREGS, data_arr[id].pdata.pid, NULL, &regs);
	data_arr[id].allocAddr = (void *)regs.rax;

	ptrace(PTRACE_SETREGS, data_arr[id].pdata.pid, NULL, &old_regs);

	return true;
}

bool pexecute(void (*method)(), size_t id) {
	if (id >= data_arr_size) {
		fputs("[PROCESS_H] out of range error\n", stderr);
		return false;
	} else if (data_arr[id].allocAddr == NULL) {
		return false;
	}
	
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
