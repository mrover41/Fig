#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/types.h>
#include <stdio.h>

#include <api/ppayload.h>
#include <api/process.h>
#include <types/pdata.h>

#include "processinf.h"

void *pfind(char *, size_t);

bool palloc(process_data *data) {
	void *addr = NULL;

	char opcode[] = {0x0F, 0x05}; //syscall
	if ((addr = pfind(opcode, sizeof(opcode) / sizeof(opcode[0]))) == NULL) {
		fputs("[PAYLOAD_H] find syscall instruction error", stderr);
		return false;
	}

#ifdef DEBUG
	fprintf(stderr, "syacall instruction found: %p", addr);
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

void *pfind(char *buff, size_t buff_size) {
	//TODO: find bytecode instruction
	
	return NULL;
}
