#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <asm/uaccess.h>


#include "sv_manage.h"
#include "svctl.h"

struct list_head dummy_node;
struct proc_dir_entry *entry;
atomic_t list_size;


int sv_register(char *sv_name, void *sv_ptr)
{
	struct sv_metadata *sv;
	sv = kmalloc(sizeof(struct sv_metadata), GFP_KERNEL);
	if(!sv){
		printk("ERROR: sv_register: kmalloc failed.\n");
		return -ENOMEM;
	}
	//TODO: need to modify to syscall_table
	sv->sv_id = atomic_read(&list_size);
	sv->sv_name = sv_name;
	sv->sv_ptr = sv_ptr;
	
	/* TODO:default will be -2 at beginning, need to be fixed */
	if(strcmp(sv_name, "default")==0)
		atomic_set(&sv->sv_rc,2);
	else
		atomic_set(&sv->sv_rc,0);
	
	list_add(&sv->node,&dummy_node);
	atomic_inc(&list_size);
	
	return 0;
}
EXPORT_SYMBOL(sv_register);


int sv_remove(char *sv_name)
{
	struct list_head *cur, *safe;
	struct sv_metadata *sv;
	
	list_for_each_safe(cur, safe, &dummy_node){
		sv = list_entry(cur, struct sv_metadata, node);
		/* check if find same vector name, if yes, then check if rc ==0 */
		if(strcmp(sv->sv_name, sv_name)==0){
			if(atomic_read(&sv->sv_rc)==0){
				list_del(cur);
				atomic_dec(&list_size);
				kfree(sv);
				return 0;
			}else{
				printk("ERROR: sv_remove: %s is in use.\n", sv_name);
				return -EBUSY;
			}
		}
	}

	printk("ERROR: sv_remove: no such vector name\n");
	return -ENOENT;
}
EXPORT_SYMBOL(sv_remove);


int sv_fork_rc(void *ptr)
{	
	struct list_head *cur;
	struct sv_metadata *sv;

	if(atomic_read(&list_size)==0){
		return 0;
	}

	list_for_each(cur, &dummy_node){
		sv = list_entry(cur, struct sv_metadata, node);
		if(sv->sv_ptr == ptr){
			atomic_inc(&sv->sv_rc);
			return 0;
		}
	}
	printk("ERROR: sv_fork_rc: no such vector ptr\n");
	return -EINVAL;

}


int sv_exit_rc(void *ptr)
{
	struct list_head *cur;
	struct sv_metadata *sv;
	
	if(atomic_read(&list_size)==0){
		return 0;
	}

	list_for_each(cur, &dummy_node){
		sv = list_entry(cur, struct sv_metadata, node);
		if(sv->sv_ptr == ptr){
			atomic_dec(&sv->sv_rc);
			return 0;
		}
	}
	printk("ERROR: sv_exit_rc: no such vector ptr\n");
	return -EINVAL;
}


static long sv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int i, total;
	void *old_ptr;
	struct sv_metadata *sv;
	struct ioctl_arg *io_arg = (struct ioctl_arg *) arg;
	struct ioctl_record *records;
	struct list_head *cur;
	struct pid *p_id;
	struct task_struct *p_task;


	switch(cmd){
		case LIST:
			/* copy kernel space value to user space pointer*/
			i = 0;
			total = atomic_read(&list_size);
			records = io_arg->records;
			
			err = copy_to_user((void *) &io_arg->sv_total, (void *) &total, sizeof(int));
			if(err){
				printk("ERROR: sv_ioctl(LIST): copy_to_user failed.\n");
				return -EFAULT;
			}

			list_for_each(cur, &dummy_node){
				sv = list_entry(cur, struct sv_metadata, node);
				if(i<total){
					
					err = copy_to_user((void *) &records[i].sv_id, (void *) &sv->sv_id, sizeof(int));
					if(err){
						printk("ERROR: sv_ioctl(LIST): copy_to_user failed.\n");
						return -EFAULT;
					}
					
					err = copy_to_user((void *) &records[i].sv_rc, (void *) &sv->sv_rc, sizeof(int));
					if(err){
						printk("ERROR: sv_ioctl(LIST): copy_to_user failed.\n");
						return -EFAULT;
					}
					
					err = copy_to_user((void *) records[i].sv_name, (void *) sv->sv_name, strlen(sv->sv_name));
					if(err){
						printk("ERROR: sv_ioctl(LIST): copy_to_user failed.\n");
						return -EFAULT;
					}
					
					i++;
				}
			}
			break;

		case CURRENT:
			/* use pid to get task_struct, then compare sv_ptr pointer*/
			p_id = find_get_pid(io_arg->p_id);
			if(!p_id){
				printk("ERROR: sv_ioctl(CURRENT): no such process.\n");
				err = -ESRCH;
				break;
			}
			p_task = pid_task(p_id, PIDTYPE_PID);
			
			list_for_each(cur, &dummy_node){
				sv = list_entry(cur, struct sv_metadata, node);
				if(sv->sv_ptr == p_task->sv_ptr){
					err = copy_to_user((void *)&io_arg->sv_id, (void *) &sv->sv_id, sizeof(int));
					if(err){
						printk("ERROR: sv_ioctl(CURRENT): copy_to_user failed.\n");
						return -EFAULT;
					}

					err = copy_to_user((void *)io_arg->sv_name, (void *) sv->sv_name, strlen(sv->sv_name));
					if(err){
						printk("ERROR: sv_ioctl(CURRENT): copy_to_user failed.\n");
						return -EFAULT;
					}
					return err;
				}
			}
			
			err = -ENODATA;
			printk("ERROR: sv_ioctl(CURRENT): no such vector.\n");
			break;

		case SET:
			/* use pid to get task_struct, then see if intented id exist, i = 1 means already found target*/
			i = 0;
			p_id = find_get_pid(io_arg->p_id);
			if(!p_id){
				printk("ERROR: sv_ioctl(SET): no such process.\n");
				err = -ESRCH;
				break;
			}
			p_task = pid_task(p_id, PIDTYPE_PID);
			old_ptr = p_task->sv_ptr;
			
			/* first round: find target vector and increase rc of new system vector*/
			list_for_each(cur, &dummy_node){
				sv = list_entry(cur, struct sv_metadata, node);
				if(io_arg->sv_id == sv->sv_id){
					if(old_ptr!=sv->sv_ptr){
						p_task->sv_ptr = sv->sv_ptr;
						atomic_inc(&sv->sv_rc);
						printk("SUCCESS: sv_ioctl(SET).\n");
						i=1;
						break;
					}else{
						printk("ERROR: sv_ioctl(SET): Already using the assigned vector.\n");
						return -EINVAL;
					}
				}
			}
			if(i==0){
				err = -EINVAL;
				printk("ERROR: sv_ioctl(SET): no such vector id\n");
				break;
			}
			/* second round: reduce rc of old system vector */
			list_for_each(cur, &dummy_node){
				sv = list_entry(cur, struct sv_metadata, node);
				if(old_ptr == sv->sv_ptr){
					atomic_dec(&sv->sv_rc);
					break;
				}
			}
			break;
	}
	return err;
}

asmlinkage long clone2(void *arg){
	return 0;
}


const struct file_operations fops = {
	.unlocked_ioctl = sv_ioctl,
};


static int __init init_sv_manage(void)
{

	int r = 0;
	entry = proc_create("sv_manage", 0644, NULL, &fops);
	if(entry == NULL){
		printk("ERROR: init_sv_manage: failed to create entry under /proc.\n");
	}
	INIT_LIST_HEAD(&dummy_node);
	atomic_set(&list_size, 0);
	/* register default vector */
	r = sv_register("default", (void*) sys_call_table);
	fork_rc_ptr = sv_fork_rc;
	exit_rc_ptr = sv_exit_rc;
	if(r<0){
		printk("ERROR: init_sv_manage: failed to register default vector.\n");
	}
	sys_clone2_ptr = clone2;
	printk("load module succeeded.\n");

	return r;
}

static void __exit exit_sv_manage(void)
{
	sys_clone2_ptr = NULL;
	exit_rc_ptr = NULL;
	fork_rc_ptr = NULL;
	sv_remove("default");
	proc_remove(entry);
	printk("unload module succeeded.\n");
}

module_init(init_sv_manage);
module_exit(exit_sv_manage);
MODULE_LICENSE("GPL");