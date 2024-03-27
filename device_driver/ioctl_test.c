#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define SHAKESPEARE_INC 1073828609
#define err_check if (ret == -1) { perror(""); return 1; }

int main() {
    int ret;
    char chars[6] = {};
    int fd = open("/dev/shakespeare", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Testing INC command on first five characters\n");
    ret = read(fd, chars, 5); 
    err_check;
    printf("Original: %s\n", chars);

    ret = ioctl(fd, SHAKESPEARE_INC, 1);
    err_check;
    ret = lseek(fd, 0, SEEK_SET);
    err_check;
    ret = read(fd, chars, 5); 
    err_check;
    printf("Incrementing by 1: %s\n", chars);

    ret = ioctl(fd, SHAKESPEARE_INC, -5);
    err_check;
    ret = lseek(fd, 0, SEEK_SET);
    err_check;
    ret = read(fd, chars, 5); 
    err_check;
    printf("Decrementing by 5: %s\n", chars);

    ret = ioctl(fd, SHAKESPEARE_INC, 4);
    err_check;
    ret = lseek(fd, 0, SEEK_SET);
    err_check;
    ret = read(fd, chars, 5); 
    err_check;
    printf("Incrementing by 4: %s\n", chars);
    return 0;
}
