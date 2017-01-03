#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/namei.h>

#include "sv_manage.h"


/* This is a vector for read-write-only */

extern int sv_register(char *, void*);
extern int sv_remove(char *);

char *name = "read-write-only";
void *my_table[__NR_syscall_max+1] = {NULL};


asmlinkage long my_create(void)
{
	printk("Forbidden: sys_create is not allowed.\n");
	return -EACCES;
}

asmlinkage long my_rename(void)
{
	printk("Forbidden: sys_rename is not allowed.\n");
	return -EACCES;
}

asmlinkage long my_unlink(void)
{
	printk("Forbidden: sys_unlink is not allowed.\n");
	return -EACCES;
}

asmlinkage long my_mkdir(void)
{
	printk("Forbidden: sys_mkdir is not allowed.\n");
	return -EACCES;
}

asmlinkage long my_rmdir(void)
{
	printk("Forbidden: sys_rmidr is not allowed.\n");
	return -EACCES;
}

asmlinkage long my_chdir(void)
{
	printk("Forbidden: sys_chdir is not allowed.\n");
	return -EACCES;
}


// asmlinkage long my_write(unsigned int fd, const char __user *buf, size_t count)
// {
// 	long (*write_ptr)(unsigned int fd_1, const char __user *buf_1, size_t count_1) = NULL;
// 	write_ptr = sys_write;

// 	if(fd==2||fd==1){
// 		return (*write_ptr)(fd, buf, count);
// 	}
// 	else{
// 		printk("Forbidden: sys_write is not allowed.\n");
// 		return -EACCES;
// 	}
// }


void sv_initialize(void)
{
	/* assign array entry to default one  */
	int i = 0;
	for(;i<__NR_syscall_max+1;i++){
		my_table[i] = sys_call_table[i];
	}

	/* assign custom function to entry */
	// my_table[__NR_write] = my_write;
	my_table[__NR_creat] = my_create;
	my_table[__NR_rename] = my_rename;
	my_table[__NR_unlink] = my_unlink;
	my_table[__NR_mkdir] = my_mkdir;
	my_table[__NR_rmdir] = my_rmdir;
	my_table[__NR_chdir] = my_chdir;

}

static int __init init_vector(void)
{
	int r = 0;
	sv_initialize();
	r= sv_register(name, (void *) my_table);
	if(r==0)
		printk("%s loaded.\n", name);
	else
		printk("ERROR: failed to load %s.\n", name);
	return r;
}


static void __exit exit_vector(void)
{
	int r = 0;
	r = sv_remove(name);
	if(r==0)
		printk("%s unloaded.\n", name);
	else
		printk("ERROR: failed to unload %s.\n", name);
}

module_init(init_vector);
module_exit(exit_vector);
MODULE_LICENSE("GPL");