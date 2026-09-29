#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <stdio.h>

#include <api/inject.h>
#include <api/process.h>
#include <types/pdata.h>

#include "processinf.h"

bool pallocate_mem(void *addr, process_data *data) {
	ppause(data);

	process_info *pinf = NULL;
	if ((pinf = get_pparent(data)) == NULL) return false;

	struct user_regs_struct old_regs, regs;
	ptrace(PTRACE_GETREGS, data->pid, NULL, &regs);

	old_regs = regs;

	regs.rip = addr;
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
	
	//TODO: Inject code

	return true;
}
