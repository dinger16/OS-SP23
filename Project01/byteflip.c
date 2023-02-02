#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int nMaxFileSize = 25000;

void bitflip(char* buffer, int length) {
    int i;
    for (i = 0; i < length; i++) {
        buffer[i] = buffer[i] ^ 0xFF;
    }
}

void reverse(char* buffer, int length) {
    int i, j = length-1, temp;
    for (i=0; i < j; i++) {
        temp = buffer[j];
        buffer[j] = buffer[i];
        buffer[i] = temp;
        j--;
    }
}

int main(int argc, char* argv[]) {

    // char a[] = {1, 2, 3, 4, 5};

    // bitflip(a, 5);

    // int i;
    // for (i=0; i<5; i++) {
    //     printf("%x ", a[i]);
    // }
    // printf("\n");
    char* filename;

    if (argc == 2) {
        filename = strdup(argv[1]);
    }

    FILE* fp = fopen(filename, "r");

    fclose(fp);

    return 0;
}