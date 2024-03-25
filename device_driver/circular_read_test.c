#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *shakespeare = fopen("/dev/shakespeare", "r");
    char* buffer = (char*) malloc(sizeof(char)*16);
    size_t buffer_size = 16;

    // Read one character
    printf("Read one character\n");
    buffer[0] = fgetc(shakespeare);
    buffer[1] = 0;
    printf("%s\n\n", buffer);
    
    // Read ten characters
    printf("Read ten characters\n");
    for (int i = 0; i < 10; i++)
        buffer[i] = fgetc(shakespeare);
    buffer[10] = 0;
    printf("%s\n\n", buffer);

    // Read one line
    printf("Read one line\n");
    getline(&buffer, &buffer_size, shakespeare);
    printf("%s\n", buffer);

    // Read ten lines
    printf("Read ten lines\n");
    for (int i = 0; i < 10; i++) { 
        getline(&buffer, &buffer_size, shakespeare);
        printf("%s", buffer);
    }

    // Read until user interrupt
    printf("\nRead until user interrupt\n");
    scanf("Ready?");
    for (;;)
        printf("%c", fgetc(shakespeare));
    return 0;
}
