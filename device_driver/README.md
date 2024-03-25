# Device Driver. Tasks and comments.
- [x] Understand the pseudo example by rewriting from scratch (commit 0c12d0b).
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
    + On device reads, I give a character at a time of a poem.
    + `od -t c -N 20`: Output readable characters with `od`.
    + Since my poem contains non-ASCII character, they occupy more than one byte and are not meaningful when inspected through `od`.
- [x] Make device capacity a module parameter (commit 0c12d0b).
    + Is now settable using `sudo insmod shakespeare.ko capacity=128`
- [x] Make reading from the device a circular operation; i.e. upon reaching the end of the device, continue from the beginning. Also, write a test program to test various cases.
    + All `*f_pos` check have been replaced with `if (*f_pos >= capacity) *f_pos = 0;`
    + `circular_read_test.c` created and Makefile now includes its compile command 
- [ ] Implement the write operation. This should not be circular and report "device full" when device end is reached. Also, write a test program to test various cases.
- [ ] Implement the seek operation. Also, write a test program to test various cases.
- [ ] Implement an ioctl operation named "PSEUDO_INC" which takes an 8-bit, signed integer value as parameter and increases each byte in the device by that value. Also, write a test program to test various cases.
- [ ] At the moment, the device node has to be created manually after inserting the module. Handle this automatically without the need for human intervention.
- [ ] Add an entry for the device under the `/proc` file system, like `/proc/pseudo`.This entry should report the device capacity.


