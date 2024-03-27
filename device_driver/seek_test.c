#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define err_check if (ret == -1) { perror(""); return 1; }

int main() {
    int fd = open("/dev/shakespeare", O_RDONLY);
    char c;
    if (fd < 0) {
        perror("open");
        return 1;
    }
    int ret;
    ret = lseek(fd, 0, SEEK_SET);
    err_check;
    read(fd, &c, 1);
    printf("First character: %c\n", c);

    ret = lseek(fd, 0, SEEK_END);
    err_check;
    read(fd, &c, 1);
    printf("Last character: %c\n", c);

    printf("Device capacity: %d\n", ret+1);

    ret /= 2;
    ret = lseek(fd, ret, SEEK_SET);
    err_check;
    read(fd, &c, 1);
    printf("Middle character: %c\n", c);

    ret = lseek(fd, ret*3, SEEK_CUR);
    err_check;
    read(fd, &c, 1);
    printf("Should not print: %c\n", c);
    return 0;
}
