# Device Driver. Tasks and comments.
- [x] Understand the pseudo example by rewriting from scratch. (commit 0c12d0b)
    + I named my program name `shakespeare.c`
    + I explicitly included all libraries and motivated its usage with a comment
    + In each **printk**, I add module's name using a macro:
    ```C
    #define THIS "[Shakespeare] "
    ...
    printk(KERN_DEBUG THIS "Lo! Unveiling apparatus.\n");
    ```
    + I did not include the option of manual major number allocation
    + I used mutex instead of semaphore.
    + buf[x] = 'A' caused scary errors that made my virtual machine freeze. I had to do a power cycle. However, copy_to_user worked perfectly.
    + LDD3, chapter 3
    > Attempting to reference the user-space memory directly could generate a page fault, which is something that kernel code is not allowed read and write to do. The result would be an “oops,” which would result in the death of the process that made the system call.
    + On device reads, I give a character at a time of a poem.
    + `od -t c -N 20`: Output readable characters with `od`.
- [x] Make device capacity a module parameter. (commit 0c12d0b)
    + Is now settable using `sudo insmod shakespeare.ko capacity=128`
- [x] Make reading from the device a circular operation; i.e. upon reaching the end of the device, continue from the beginning. Also, write a test program to test various cases. (commit f0f7d7d)
    + All `*f_pos` check have been replaced with `if (*f_pos >= capacity) *f_pos = 0;`
    + `read_test.c` created and Makefile now includes its compile command 
- [x] Implement the write operation. This should not be circular and report "device full" when device end is reached. Also, write a test program to test various cases. (commit c6099c7)
    + Writing requires sudo permissions. I debugged my program by checking for return values of **open** and **write** and printing errno with **perror**.
    ```
    davidoglu@prison:~/sysprog_hw/device_driver
    $ ./write_test 
    open: Permission denied
    davidoglu@prison:~/sysprog_hw/device_driver
    $ sudo ./write_test
    (worked!)
    ```
    + I got an infinite B machine because I was returning 0 in my device's write function. Functions in standard c libraries try to write again untile they receive a return value telling them that the operation succeeded.
    + Again an issue related to the return value. I did not account for the case when user tries to exceed the device's capacity. This must be handled with a proper error code. 
    + From errno(3):
    > ENOMEM Not enough space/cannot allocate memory 
    + I am successful at writing, however I cannot check for the return error code with sophisticated C functions like **fputc**, **fputs** etc.
- [x] Implement the seek operation. Also, write a test program to test various cases. (commit 1cb4efd)
- [x] Implement an ioctl operation named "PSEUDO_INC" which takes an 8-bit, signed integer value as parameter and increases each byte in the device by that value. Also, write a test program to test various cases. (commit 31b58be)
> $ ./ioctl_test
Testing INC command on first five characters
Original: Shall
Incrementing by 1: Tibmm
Decrementing by 5: Od]hh
Incrementing by 4: Shall
- [x] At the moment, the device node has to be created manually after inserting the module. Handle this automatically without the need for human intervention. (commit 84ed6e3)
- [x] Add an entry for the device under the `/proc` file system, like `/proc/pseudo`.This entry should report the device capacity. (commit dfe942b)
    + When I do `cat /proc/shakespeare`, it prints the information without stopping. I hope this is the expected behaviour.


