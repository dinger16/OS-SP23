#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void check_num_args(int argc) {
    // there should be three arguments: the program name, the filename, and the lookup byte
    if (argc != 3) {
        fprintf(stderr, "Error: Incorrect number of inputs!\n");
        exit(1);
    }
}

void check_hex(char* hex) {
    // the lookup hex byte should be in the form '0x##'
    if (hex[0] != '0' || hex[1] != 'x') {
        fprintf(stderr, "Error: Invalid lookup byte!\n");
        exit(1);
    }
}

void check_lookup_byte(long int lookup_byte, char* end) {
    // the lookup byte should be a valid hex digit
    if (*end != '\0') {
        fprintf(stderr, "Error: Invalid lookup byte!\n");
        exit(1);
    }
    // the lookup byte should be less than 0x99
    if (lookup_byte > 0x99) {
        fprintf(stderr, "Error: Hex value too large!\n");
        exit(1);
    }
}

struct stat check_file_stats(char* filename) {
    struct stat sb;
    // the file should exist, be a regular file, and be less than 25kB
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

    return sb;
}

void check_readability(char* filename) {
    // the file should be readable
    if (access(filename, R_OK) != 0) {
        fprintf(stderr, "Error: Unable to open! File was not readable.\n");
        exit(1);
    }
}

void check_size(struct stat sb, int result) {
    // if read correctly, the number of bytes read should equal the file size
    if (result != sb.st_size) {
        fprintf(stderr, "Error: Reading error");
    }
}

void count_lookup_byte(char* buffer, struct stat sb, long int lookup_byte) {
    // iterate through all the bytes in the buffer and count the number of itmes the lookup byte appears
    int i, count = 0;
    for (i = 0; i < sb.st_size; i++) {
        if (buffer[i] == lookup_byte) {
            count++;
        }
    }
    printf("%d\n", count);
}

int main(int argc, char* argv[]) {

    char* filename;
    char* end;
    long int lookup_byte;

    // Check the user inputter the correct number of arguments, a valid hex digit as a lookup byte, and a valid file
    check_num_args(argc);
    check_hex(argv[2]);
    lookup_byte = strtol(argv[2], &end, 16);
    check_lookup_byte(lookup_byte, end);
    filename = strdup(argv[1]);
    struct stat sb = check_file_stats(filename);

    // Check the file is readable and if so then open it, read it into a buffer, and check the file was read correctly
    check_readability(filename);
    FILE* fp = fopen(filename, "r");
    char buffer[sb.st_size];
    size_t result = fread(buffer, sizeof(char), sb.st_size, fp);
    check_size(sb, result);

    // Count the number of times the lookup byte appears in the file and print it
    count_lookup_byte(buffer, sb, lookup_byte);

    return 0;
}