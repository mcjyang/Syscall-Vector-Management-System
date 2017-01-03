#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/namei.h>

#include "sv_manage.h"


/* This is a vector for delete forbidden */

extern int sv_register(char *, void*);
extern int sv_remove(char *);

char *name = "do-not-delete";
void *my_table[__NR_syscall_max+1] = {NULL};



asmlinkage long my_unlink(void)
{
	printk("Forbidden: sys_unlink is not allowed.\n");
	return -EACCES;
}


asmlinkage long my_rmdir(void)
{
	printk("Forbidden: sys_rmdir is not allowed.\n");
	return -EACCES;
}


void sv_initialize(void)
{
	/* assign array entry to default one  */
	int i = 0;
	for(;i<__NR_syscall_max+1;i++){
		my_table[i] = sys_call_table[i];
	}

	/* assign custom function to entry */
	my_table[__NR_unlink] = my_unlink;
	my_table[__NR_rmdir] = my_rmdir;
	
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