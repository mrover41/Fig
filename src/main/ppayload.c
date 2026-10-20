#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdio.h>

#include <api/ppayload.h>
#include <api/process.h>
#include <types/pdata.h>

#include "processinf.h"


void *pfind(const char *, size_t, process_data *);
ssize_t get_msize(void *);

bool palloc(process_data *data) {
	void *addr = NULL;

#ifdef DEBUG
	puts("Try to alloc memory in target process\n");
#endif

	char opcode[] = {0x0F, 0x05}; //syscall
	if ((addr = pfind(opcode, sizeof(opcode) / sizeof(opcode[0]), data)) == NULL) {
		fputs("[PAYLOAD_H] find syscall instruction error\n", stderr);
		return false;
	}

#ifdef DEBUG
	fprintf(stderr, "syacall instruction found: %p\n", addr);
#endif

	ppause(data);

	process_info *pinf = NULL;
	if ((pinf = get_pparent(data)) == NULL) return false;

	struct user_regs_struct old_regs, regs;
	ptrace(PTRACE_GETREGS, data->pid, NULL, &regs);

	old_regs = regs;

	regs.rip = (long long unsigned int)addr;
	regs.rax = 0x09;
	regs.rdi = 0;
	regs.rsi = 4096;
	regs.rdx = 0x1 | 0x2 | 0x4; //READ|WRITE|EXEC
	regs.r10 = 0x02 | 0x20; //PRIVATE|ANONYMOUS
	regs.r8  = (unsigned long long)-1;
	regs.r9  = 0;

	regs.orig_rax = -1; //Kernel can save RAX before interrupt syscall, we must reset this

	ptrace(PTRACE_SETREGS, data->pid, NULL, &regs);
	ptrace(PTRACE_SINGLESTEP, data->pid, NULL, NULL);

	ptrace(PTRACE_GETREGS, data->pid, NULL, &regs);
	pinf->allocAddr = (void *)regs.rax;

	ptrace(PTRACE_SETREGS, data->pid, NULL, &old_regs);

	return true;
}

/*bool pexecute(void (*method)(), process_data *data) {
	process_info *pinf = NULL;
	if ((pinf = get_pparent(data)) == NULL) return false;
	if (pinf->allocAddr == NULL) return false;
	
	ssize_t msize;
	if ((msize = get_msize(method)) < 0) return false;

	struct iovec local_iov[1];
	local_iov[0].iov_base = method;
	local_iov[0].iov_len = msize;

	struct iovec remote_iov[1];
	remote_iov[0].iov_base = pinf->allocAddr;
	remote_iov[0].iov_len = msize;

	if (process_vm_writev(data->pid, local_iov, 1, remote_iov, 1, 0) != msize) return false;

	//TODO: run injected code in remote process

	return true;
}*/

void *pfind(const char *buff, size_t buff_size, process_data *dat) { //TODO: test and fix find bytecode instruction
	size_t bsize = dat->base_end - dat->base;
	char *membuff = malloc(bsize);

#ifdef DEBUG
	fprintf(stdout, "Try to find instruction\nLocal memory buffer size: %li\nInstruction buffer size: %li\nProcess PID: %i\n\n", bsize, buff_size, dat->pid);
#endif

	struct iovec local_iov[1];
	local_iov[0].iov_base = membuff;
	local_iov[0].iov_len = bsize;

	struct iovec remote_iov[1];
	remote_iov[0].iov_base = dat->base;
	remote_iov[0].iov_len = bsize;

	ssize_t nread = process_vm_readv(dat->pid, local_iov, 1, remote_iov, 1, 0);
	if (nread < 0) {
		perror("process_vm_readv failed");
		fprintf(stderr, "[PFIND] Err: nread < 0\nnread = %li\n", nread);
        free(membuff);
        return NULL;
    }

	if (bsize < buff_size) {
		free(membuff);
		fputs("[PFIND] Err: bsize < buff_size\n", stderr);
		return NULL;
	}

	void *res = NULL;

    for (size_t i = 0; i <= (size_t)nread - buff_size; i++) {
        if (memcmp(membuff + i, buff, buff_size) == 0) {
            res = (void *)(dat->base + i);
            break;
        }
    }
	
#ifdef DEBUG
	fprintf(stdout, "Instruction found, adress: %p\n", res);
#endif

	free(membuff);
	return (void *)res;
}

ssize_t get_msize(void *method) {
	size_t indx = 0;
	while (((unsigned char*)method)[indx] != 0xC3) {
		indx++;
	}
	return indx;
}
