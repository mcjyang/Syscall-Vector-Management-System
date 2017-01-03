#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "../svctl.h"

// #ifndef __NR_sv_manage
// #error sv_manage system call not defined
// #endif

int main(int argc, char * const argv[]){

	int rc, fd, opt, i;
	struct ioctl_arg arg;
	struct ioctl_record *records;
	
	if(argc<2){
		printf("ERROR: main: not enough arguments.\n");
		exit(1);
	}

	fd = open("/proc/sv_manage", O_RDONLY, 0);
	if(fd < 0){
		printf("ERROR: main: failed to create fd.\n");
		exit(1);
	}

	if((opt = getopt(argc, argv, "ls:c:")) != -1){
		switch(opt){
			case 'l':
				/* list existed vectors */
				i = 0;
				/* initialize char array in struct */
				for(;i<SV_NUM_LIMIT;i++){
					memset(arg.records[i].sv_name,'\0', sizeof(char)*NAME_LEN_LIMIT);
				}
				/* ioctl: list */
				rc = ioctl(fd, LIST, &arg);
				
				/* print out result */
				i = arg.sv_total-1;
				records = arg.records;
				printf("============= Vector List =============\n");
				while(i>=0){
					printf("id: %d    name: %15.30s    rc: %d\n",records[i].sv_id, records[i].sv_name,records[i].sv_rc);
					i--;
				}
				printf("================= End =================\n");
				break;

			case 's':
				arg.p_id = atoi(optarg);
				arg.sv_id = atoi(argv[optind]);
				rc = ioctl(fd, SET, &arg);
				if(rc){
					printf("Failed: bad operation, please try again.\n");
				}
				break;
			case 'c':
				arg.p_id = atoi(optarg);
				rc = ioctl(fd, CURRENT, &arg);
				printf("Vector INFO:  id: %d    name: %s\n", arg.sv_id, arg.sv_name);
				break;
			case '?':
				printf("ERROR: main: option not support.\n");
				break;
		}
	}		

	exit(rc);
}