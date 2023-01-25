#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error: Incorrect number of inputs!\n");
        exit(1);
    }
    char* filename;
    char* end;
    long int lookup_byte;

    // Check if hex value starts with '0x'
    if (argv[2][0] != '0' || argv[2][1] != 'x') {
        fprintf(stderr, "Error: Invalid lookup byte!\n");
        exit(1);
    }

    
    lookup_byte = strtol(argv[2], &end, 16);
    if (*end != '\0') {
        fprintf(stderr, "Error: Invalid lookup byte!\n");
        exit(1);
    }


    if (lookup_byte > 0x99) {
        fprintf(stderr, "Error: Hex value too large!\n");
        exit(1);
    }

    filename = strdup(argv[1]);
    struct stat sb;
    if (stat(filename, &sb) != 0) {
        fprintf(stderr, "Error: File does not exist!\n");
        exit(1);
    }
    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "Error: Unable to open! Object was not a file.\n");
        exit(1);
    }

    if (sb.st_size > 25000) {
        fprintf(stderr, "Error: The file is over 25kB (file size was %lld bytes)\n", (long long) sb.st_size);
        exit(1);
    }

    /* Fix permission issue later */

    // if (!(sb.st_mode & S_IRUSR)) {
    //     fprintf(stderr, "You do not have permission to read this file!\n");
    //     exit(1);
    // }

    FILE* fp = fopen(filename, "r");
    char buffer[sb.st_size];
    size_t result = fread(buffer, sizeof(char), sb.st_size, fp);
    if (result != sb.st_size) {
        fprintf(stderr, "Error: Reading error");
    }

    int i, count = 0;
    for (i = 0; i < sb.st_size; i++) {
        if (buffer[i] == lookup_byte) {
            count++;
        }
    }
    printf("%d\n", count);

    return 0;
}