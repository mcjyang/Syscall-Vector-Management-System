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


#include "../svctl.h"

/* this is the test case for read-only vector and ioctl support */
int main(int argc, char * const argv[]){

	int r, fd, i;
	char buf[10] = "";
	struct ioctl_arg arg;
	struct ioctl_record *records;

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd<0){
		printf("ERROR: main: failed to create fd.\n");
		exit(1);
	}

	arg.p_id = (int)getpid();
	printf("This process PID: %d\n", arg.p_id);
	/* before assignment */
	r = ioctl(fd, CURRENT, &arg);
	printf("BEFORE ASSIGNMENT:  id: %d    name: %s\n", arg.sv_id, arg.sv_name);

	/* assign to read only sv */
	arg.sv_id = 1;
	r = ioctl(fd, SET, &arg);
	if(r){
		printf("ERROR: bad operation, please try again.\n");
	}

	/* after assignement */
	r = ioctl(fd, CURRENT, &arg);
	printf("AFTER ASSIGNMENT:  id: %d    name: %s\n", arg.sv_id, arg.sv_name);

	/* list current sv status*/
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

	fd = creat("test.txt", O_CREAT);
	if(fd<0){
		printf("ERROR: create err\n");
	}
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


	/* assign to default */
	arg.p_id = (int)getpid();
	arg.sv_id = 0;

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd<0){
		printf("ERROR: main: failed to create fd.\n");
		exit(1);
	}

	r = ioctl(fd, SET, &arg);
	if(r){
		printf("ERROR: bad operation, please try again.\n");
	}

	/* list current sv status*/
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

	
	fd = creat("test.txt", O_CREAT);
	if(fd<0){
		printf("ERROR: create err\n");
	}
	close(fd);

	r = unlink("test.txt");
	if(r<0){
		printf("ERROR: unlink err\n");
	}

	exit(0);
}