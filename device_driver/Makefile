obj-m := shakespeare.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g read_test.c -o read_test
	gcc -g write_test.c -o write_test
	gcc -g seek_test.c -o seek_test
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
