obj-m += svctl.o
obj-m += test_1.o
obj-m += test_2.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include

default: svctl test_1 test_2

svctl: svctl.c
	gcc -Wall -Werror -I$(INC)/generated/uapi -I$(INC)/uapi svctl.c -o svctl

test_1: test_1.c
	gcc -Wall -Werror -I$(INC)/generated/uapi -I$(INC)/uapi test_1.c -o test_1

test_2: test_1.c
	gcc -Wall -Werror -I$(INC)/generated/uapi -I$(INC)/uapi test_2.c -o test_2

clean: 
	rm -f svctl test_1 test_2
	rmdir test