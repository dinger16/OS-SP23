#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void exit_handler(int signum) {
    fprintf(stderr, "\nControl-C was pressed... exiting\n");
    exit(1);
}

int input_size = 100;
int max_command_len = 10;

int main(int argc, char* argv[]) {

    if (argc != 1) {
        fprintf(stderr, "Error: Executable does not take any parameters!\n");
        exit(1);
    }

    signal(SIGINT, exit_handler);

    char input[input_size];

    fprintf(stdout, "Execute? ");
    fgets(input, input_size, stdin);

    input[strlen(input) - 1] = '\0';
    char* split = strtok(input, " ");
    char* command[max_command_len];

    int counter = 0;
    while (split != NULL) {
        command[counter] = split;
        counter++;
        split = strtok(NULL, " ");
    }
    command[counter] = NULL;

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    }
    else if (rc == 0) {
        fprintf(stdout, "Executing: %s\n", command[0]);
        execvp(command[0], command);
        fprintf(stderr, "%s failed to run... Enter a valid command!\n", command[0]);
        exit(-1);
    }
    else {
        wait(NULL);
        fprintf(stdout, "Execution complete\n");
    }

    return 0;
}