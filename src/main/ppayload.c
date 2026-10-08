#define _GNU_SOURCE

#include <sys/ptrace.h>
#include <sys/types.h>
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

bool palloc(process_data *data) {
	void *addr = NULL;

#ifdef DEBUG
	puts("Try to alloc memory in target process\n");
#endif

	char opcode[] = {0x0F, 0x05}; //syscall
	if ((addr = pfind(opcode, sizeof(opcode) / sizeof(opcode[0]), data)) == NULL) { //TODO: Убрать это нахуй ибо оно не работает
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
	regs.rdx = 0x1 | 0x2 | 0x4;
	regs.r10 = 0x02 | 0x20;
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

bool pexecute(void (*method)(), process_data *data) {
	process_info *pinf = NULL;
	if ((pinf = get_pparent(data)) == NULL) return false;

	if (pinf->allocAddr == NULL) return false;
	
	//TODO: Inject code (target process must execute method [in own adress spece and use own stack])

	return true;
}

void *pfind(const char *buff, size_t buff_size, process_data *dat) { //TODO: find bytecode instruction
	size_t bsize = dat->base_end - dat->base;
	char *membuff = malloc(bsize);

#ifdef DEBUG
	fprintf(stdout, "Try to find instruction, memory buffer size: %li\nBuffer size: %li\n\n", bsize, buff_size);
#endif

	struct iovec local_iov[1];
	local_iov[0].iov_base = membuff;
	local_iov[0].iov_len = bsize;

	struct iovec remote_iov[1];
	remote_iov[0].iov_base = dat->base;
	remote_iov[0].iov_len = bsize;

	ssize_t nread = process_vm_readv(dat->pid, local_iov, 1, remote_iov, 1, 0);

	size_t res = 0;
	
	if (bsize < buff_size) {
		free(membuff);
		fputs("[PFIND] Err: bsize >= buff_size", stderr);
		return NULL;
	}

	void *found_addr = NULL;

    for (size_t i = 0; i <= bsize - buff_size; i++) {
        if (memcmp(membuff + i, buff, buff_size) == 0) {
            found_addr = (void *)(dat->base + i);
            break;
        }
    }
	
#ifdef DEBUG
	fprintf(stdout, "Instruction found, adress: %li\n", res);
#endif

	free(membuff);
	return (void *)res;
}
