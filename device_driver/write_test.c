#include <stdio.h>

int main() {
    int i = 0;
    FILE *shakespeare = fopen("/dev/shakespeare", "w");
    const char new_poem[] = ""
        "But thy eternal summer shall not fade,\n"
        "Nor lose possession of that fair thou ow’st,\n"
        "Nor shall death brag thou wand'rest in his shade,\n"
        "When in eternal lines to Time thou grow'st.\n";

    if (shakespeare == NULL) {
        perror("fopen");
        return 1;
    }

    printf("Writing one character\n");
    if (fputc(new_poem[0], shakespeare) <= 0) {
        printf("Error, unable to write\n");
        fclose(shakespeare);
        return 1;
    }

    printf("Writing ten characters\n");
    for (i = 1; i <= 10; i++)
        if (fputc(new_poem[i], shakespeare) <= 0) {
            printf("Error, unable to write\n");
            fclose(shakespeare);
            return 1;
        }

    printf("Writing the whole poem\n");
    if (fputs(new_poem, shakespeare) <= 0) {
        printf("Error, unable to write\n");
        fclose(shakespeare);
        return 1;
    }

    printf("Writing done\n");
    return 0;
    fclose(shakespeare);
}
