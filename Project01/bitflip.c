#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

long int nMaxFileSize = 25000;
char* userRequest = "-bf";
char* out_filename = NULL;
int given_out_filename = 0;
char* in_filename = NULL;

void check_num_args(int argc) {
    // there should be three arguments: the program name, the filename, and the lookup byte
    if (argc < 2) {
        fprintf(stderr, "Error: Too few inputs!\n");
        exit(1);
    }
}

void process_args(int argc, char* argv[]) {

    int i = 1;
    int request_count = 0;
    int maxsize_count = 0;
    int out_filename_count = 0;
    int in_filename_count = 0;

    while (i < argc) {
        
        if (strcmp(argv[i], "-help") == 0) {
            fprintf(stderr, "Potential Arguments:\n");
            fprintf(stderr, "\t-maxsize XXX: Set the maximum file size to XXX bytes.\n");
            fprintf(stderr, "\t-o XXX: Set the output file name to XXX.\n");
            fprintf(stderr, "\t-r: Return ther reverse order of bytes.\n");
            fprintf(stderr, "\t-bf: Return the bit-flipped order of bytes.\n");
            fprintf(stderr, "\t-help: Print this help message.\n");
            exit(1);
        } else if (strcmp(argv[i], "-bfr") == 0 || strcmp(argv[i], "-r") == 0) {
            userRequest = argv[i];
            request_count++;

            // check if user enterred a request multiple times: i.e. -bfr and -r in the same command line call
            if (request_count > 1) {
                fprintf(stderr, "Error: Too many user requests!\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-maxsize") == 0) {
            
            i++;
            if (i == argc) {
                fprintf(stderr, "Error: Too few inputs!\n");
                exit(1);
            }
            char* endptr = NULL;
            nMaxFileSize = strtol(argv[i], &endptr, 10);
            // check if the user entered a valid integer
            if (*endptr != '\0') {
                fprintf(stderr, "Error: Invalid maxsize argument!\n");
                exit(1);
            }
            if (nMaxFileSize < 0) {
                fprintf(stderr, "Error: Invalid maxsize argument!\n");
                exit(1);
            } else if (nMaxFileSize > 100000000) {
                fprintf(stderr, "Error: Maxsize exceeds 100,000,000 bytes!\n");
                exit(1);
            }

            maxsize_count++;

            // check if user enterred a request multiple times: i.e. -bfr and -r in the same command line call
            if (maxsize_count > 1) {
                fprintf(stderr, "Error: Too many maxsize arguments!\n");
                exit(1);
            }

        } else if (strcmp(argv[i], "-o") == 0) {
            i++;

            if (i == argc) {
                fprintf(stderr, "Error: Too few inputs!\n");
                exit(1);
            }

            out_filename = strdup(argv[i]);
            given_out_filename = 1;

            out_filename_count++;
            
            if (out_filename_count > 1) {
                fprintf(stderr, "Error: Too many output filename arguments!\n");
                exit(1);
            }

        } else if (argv[i][0] != '-') {
            in_filename = strdup(argv[i]);
            in_filename_count++;

            if (in_filename_count > 1) {
                fprintf(stderr, "Error: Too many input filename arguments!\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Potential Arguments:\n");
            fprintf(stderr, "\t-maxsize XXX: Set the maximum file size to XXX bytes.\n");
            fprintf(stderr, "\t-o XXX: Set the output file name to XXX.\n");
            fprintf(stderr, "\t-r: Return ther reverse order of bytes.\n");
            fprintf(stderr, "\t-bf: Return the bit-flipped order of bytes.\n");
            fprintf(stderr, "\t-help: Print this help message.\n");
            exit(1);
        }

        i ++;
    
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

    if (sb.st_size > nMaxFileSize) {
        fprintf(stderr, "Error: The file is over %ld bytes (file size was %lld bytes)\n", nMaxFileSize, (long long) sb.st_size);
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

void bitflip(char* buffer, int length) {
    // flip every bit in buffer with XOR
    int i;
    for (i = 0; i < length; i++) {
        buffer[i] = buffer[i] ^ 0xFF;
    }
}

void reverse(char* buffer, int length) {
    // reverse the order of the buffer by swapping the first and last bytes, then the second and second to last, etc.
    int i, j = length-1, temp;
    for (i=0; i < j; i++) {
        temp = buffer[j];
        buffer[j] = buffer[i];
        buffer[i] = temp;
        j--;
    }
}

void write_to_file(char* buffer, int length, char* filename) {

    struct stat sb;
    // the file should not exist
    if (stat(filename, &sb) == 0) {
        fprintf(stderr, "Error: %s file already exist!\n", filename);
        exit(1);
    }

    // write buffer to the output file
    FILE* fp = fopen(filename, "w");

    fwrite(buffer, sizeof(char), length, fp);
    fclose(fp);
}

int main(int argc, char* argv[]) {

    check_num_args(argc); // check the number of arguments
    process_args(argc, argv); // process the user's input

    // initialize input and ouput file names
    //char* in_filename;
    
    //in_filename = strdup(argv[1]);
    if (!in_filename) {
        fprintf(stderr, "Error: No input file specified!\n");
        exit(1);
    }
    
    struct stat sb = check_file_stats(in_filename); // checks for object's existence, type, and size
    check_readability(in_filename); // checks for read permissions

    FILE* fp = fopen(in_filename, "r");
    char* buffer = (char*) malloc(sb.st_size * sizeof(char)); // allocate buffer according to filesize
    size_t result = fread(buffer, sizeof(char), sb.st_size, fp);
    check_size(sb, result); // check the read was successful

    // reverse and/or bitflip the buffer based on the user's input and assign appropriate extension
    char* extension;
    if (strcmp(userRequest, "-bf") == 0) {
        bitflip(buffer, sb.st_size);
        extension = ".bf";
    } else if (strcmp(userRequest, "-r") == 0) {
        reverse(buffer, sb.st_size);
        extension = ".r";
    } else {
        bitflip(buffer, sb.st_size);
        reverse(buffer, sb.st_size);
        extension = ".bfr";
    }

    // allocate memory for ouput file and add the appropriate extension
    if (given_out_filename == 0) {
        out_filename = strdup(in_filename);
        out_filename = strcat(out_filename, extension);
    }

    write_to_file(buffer, sb.st_size, out_filename); // write the buffer to the output file

    fclose(fp); // close the input file
    free(buffer); // free the buffer

    fprintf(stdout, "Input: %s was %d bytes\n", in_filename, (int) sb.st_size);
    fprintf(stdout, "Output: %s was output successfully\n", out_filename);

    free(out_filename);
    free(in_filename);

    return 0;
}