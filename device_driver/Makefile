obj-m := shakespeare.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g circular_read_test.c -o circular_read_test
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
