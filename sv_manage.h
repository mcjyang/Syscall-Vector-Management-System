#include <linux/list.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <asm/syscall.h>
#include <asm/unistd.h>
#include <asm/asm-offsets.h>  // __NR_syscall_max+1


asmlinkage extern long (*sys_clone2_ptr)(void *arg);

extern const sys_call_ptr_t sys_call_table[];
extern void *fork_rc_ptr;
extern void *exit_rc_ptr;

struct sv_metadata{
	int sv_id;
	char *sv_name;
	void *sv_ptr;
	atomic_t sv_rc;
	struct list_head node;
};
