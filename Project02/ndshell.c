#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define _XOPEN_SOURCE 600

#define maxChildProcesses 1000
#define maxInputSize  100
#define maxCommandLen 10

int numChildProcesses = 0;
int childProcesses[maxChildProcesses];

void signalCapture(int signum) {
    fprintf(stderr, "\nControl-C was pressed... exiting\n");
    exit(1);
}

void clearBuffer(char** buffer, int length) {
    int i;
    for (i = 0; i < length; i++) {
        buffer[i] = NULL;
    }
}

void createChildProcess(char** command) {

    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "Fork Failed\n");
        // shouldn't exit anymore
        exit(1);
    }
    else if (rc == 0) {
        // fprintf(stdout, "Executing: %s\n", command[0]);
        execvp(command[0], command); // catch execvp because also shoudn't exit
    }
    else {
        int cpid = rc;
        childProcesses[numChildProcesses] = cpid;
        numChildProcesses++;
        printf("Process %d started\n", cpid);
        fflush(stdout);
    }
}

int isChildProccess(int cpid) {
    int i;
    for (i = 0; i < numChildProcesses; i++) {
        if (cpid == childProcesses[i])
            return 1;
    }
    return 0;
}

void removeChildProcess(int cpid) {
    int temp_array[numChildProcesses - 1];

    int i, counter = 0;
    for (i = 0; i < numChildProcesses; i++) {
        if (cpid != childProcesses[i]) {
            temp_array[counter] = childProcesses[i];
            counter++;
        }
    }

    for (i = 0; i < numChildProcesses - 1; i++) {
        childProcesses[i] = temp_array[i];
    }
    childProcesses[numChildProcesses] = 0;
}

int main(int argc, char* argv[]) {

    if (argc != 1) {
        fprintf(stderr, "Error: Executable does not take any parameters!\n");
        exit(1);
    }

    struct sigaction sigSetValue;
    sigSetValue.sa_handler = signalCapture;
    // catch control-c and send to signalCapture
    sigaction(SIGINT, &sigSetValue, NULL);

    const int ppid = getpid();


    int flag = 1;
    while (flag && ppid == getpid()) {

        // printf("%d", getpid());

        // handling input from user
        char input[maxInputSize];

        fprintf(stdout, "ndshell>");
        fgets(input, maxInputSize, stdin);

        input[strlen(input) - 1] = '\0';
        char* split = strtok(input, " ");
        char* command[maxCommandLen];
        char* executable[maxCommandLen - 1];

        int counter = 0;
        while (split != NULL && counter < maxCommandLen) {
            if (counter > 0) {
                executable[counter - 1] = split;
            }
            command[counter] = split;
            split = strtok(NULL, " ");
            counter++;
        }
        command[counter] = NULL;
        executable[counter - 1] = NULL;


        // if newline is pressed
        if (counter == 0) {
            continue;
        }

        int wstatus;
        if (strcmp(command[0], "exit") == 0) {
            // kill all child processes
            flag = 0;
        }
        else if (strcmp(command[0], "start") == 0) {
            // call fork and exec and keep track of cpid
            createChildProcess(executable);
        }
        else if (strcmp(command[0], "wait") == 0) {
            // wait for any child process to exit
            if (numChildProcesses == 0) {
                printf("No children\n");
            }
            else {
                int cpid = waitpid(-1, &wstatus, 0);
                if (wstatus == WIFSIGNALED(wstatus)) {
                    if (WTERMSIG(wstatus)) {
                        printf("Process %d exited abnormally with signal %d\n", cpid, WTERMSIG(wstatus));
                    }
                    else {
                        printf("Process %d exited normally with status %d\n", cpid, WEXITSTATUS(wstatus));
                    }
                }
                fflush(stdout);
                removeChildProcess(cpid);
                numChildProcesses--;
            }
        }
        else if (strcmp(command[0], "waitfor") == 0) {
            // make sure counter is greater than 1
            // get the specified cpid
            // wait for specific cpid
            int cpid = atoi(command[1]);

            if (counter <= 1 || cpid == 0) {
                fprintf(stderr, "Error: Invalid waitfor command! Invalid pid specified.\n");
            }
            else if (numChildProcesses == 0) {
                printf("No children\n");
            }
            else if (!isChildProccess(cpid)) {
                fprintf(stderr, "Error: Process %d does not exist!\n", cpid);
            }
            else {
                waitpid(cpid, &wstatus, 0);
                if (wstatus == WIFSIGNALED(wstatus)) {
                    if (WTERMSIG(wstatus)) {
                        printf("Process %d exited abnormally with signal %d\n", cpid, WTERMSIG(wstatus));
                    }
                    else {
                        printf("Process %d exited normally with status %d\n", cpid, WEXITSTATUS(wstatus));
                    }
                }
                fflush(stdout);
                removeChildProcess(cpid);
                numChildProcesses--;
            }
        }
        else if (strcmp(command[0], "run") == 0) {
            // run start and waitfor for given process
        }
        else if (strcmp(command[0], "kill") == 0) {
            // make sure counter is greater than 1
            // get the specified cpid
            // kill specific cpid
        }
        else if (strcmp(command[0], "quit") == 0) {
            // wait for all child processes
            // exit gracefully
        }
        else if (strcmp(command[0], "bound") == 0) {
            // run with a time limit and if time limit exceeded then kill child process
        }
        else {
            printf("Please enter a valid command!\n");
        }

        // clearBuffer(command, counter);
        // clearBuffer(executable, counter - 1);

    }
    return 0;
}