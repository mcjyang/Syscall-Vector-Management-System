obj-m += sv_manage.o
obj-m += vector_1.o
obj-m += vector_2.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include

all: sv_manage vector_1 vector_2

sv_manage:
	make -Wall -Werror -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

vector_1:
	make -Wall -Werror -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules	

vector_2:
	make -Wall -Werror -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules	

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean