#define _GNU_SOURCE
#include <sched.h>
#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#include "../svctl.h"


int fn(void *p){

	int fd, i;
	struct ioctl_arg arg;
	struct ioctl_record *records;
	
	arg.p_id = (int)getpid();
	printf("INFO: Child process PID: %d\n", arg.p_id);

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd<0){
		printf("ERROR: main: failed to create fd.\n");
		return 0;
	}

	/* list current sv status*/
	i = 0;
	for(;i<SV_NUM_LIMIT;i++){
		memset(arg.records[i].sv_name,'\0', sizeof(char)*NAME_LEN_LIMIT);
	}
	i = ioctl(fd, LIST, &arg);
	i = arg.sv_total-2;
	records = arg.records;
	printf("============= Vector List =============\n");
	while(i>=0){
		printf("id: %d    name: %15.30s    rc: %d\n",records[i].sv_id, records[i].sv_name,records[i].sv_rc);
		i--;
	}
	printf("================= End =================\n");

	return 0;
}


/* this is the test case for don't delete vector and clone flags support */
/* CLONE_SYSCALLS 0x00001000 */
int main(int argc, char * const argv[]){
	
	int r, fd, i;
	char buf[10] = "";
	struct ioctl_arg arg;
	struct ioctl_record *records;
	void * stack = malloc(1024*1024);

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd<0){
		printf("ERROR: main: failed to create fd.\n");
		exit(1);
	}

	arg.p_id = (int)getpid();
	printf("INFO: Parent process PID: %d\n", arg.p_id);

	// /* assign to dont-delete sv */
	arg.sv_id = 2;
	r = ioctl(fd, SET, &arg);
	if(r){
		printf("ERROR: bad operation, please try again.\n");
	}

	/* after assignement */
	r = ioctl(fd, CURRENT, &arg);
	printf("AFTER ASSIGNMENT:  id: %d    name: %s\n", arg.sv_id, arg.sv_name);
	printf("INFO: Now test new clone flag support ...\n");

	r = clone(fn, stack+(1024*1024), 0x00001000|SIGCHLD, NULL);
	
	if(r<0){
		printf("ERROR: Unable to create the child process.\n");
        exit(1);
	}

	wait(NULL);
	free(stack);
	printf("INFO: Child process terminated.\n");

	/* test vector effectiveness */
	fd = open("input.txt", O_RDWR);
	if(fd<0){
		printf("ERROR: open err\n");
	}
	
	r = read(fd, buf, 5);
	if(r<0){
		printf("ERROR: read error\n");
	}
	printf("INFO: read: %s\n", buf);
	close(fd);

	r = unlink("input.txt");
	if(r<0){
		printf("ERROR: unlink err\n");
	}

	r = mkdir("test",0777);
	if(r<0){
		printf("ERROR: mkdir err\n");
	}

	r = rmdir("test");
	if(r<0){
		printf("ERROR: rmdir err\n");
	}

	/* list current sv status*/

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd<0){
		printf("ERROR: main: failed to create fd.\n");
		exit(1);
	}

	i = 0;
	for(;i<SV_NUM_LIMIT;i++){
		memset(arg.records[i].sv_name,'\0', sizeof(char)*NAME_LEN_LIMIT);
	}

	r = ioctl(fd, LIST, &arg);
	i = arg.sv_total-2;
	records = arg.records;
	printf("============= Vector List =============\n");
	while(i>=0){
		printf("id: %d    name: %15.30s    rc: %d\n",records[i].sv_id, records[i].sv_name,records[i].sv_rc);
		i--;
	}
	printf("================= End =================\n");


	exit(0);
}