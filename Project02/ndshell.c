#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define maxChildProcesses 1000
#define maxInputSize  100
#define maxCommandLen 10

int numChildProcesses = 0;
int runCPID = 0;
int childProcesses[maxChildProcesses];

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
        printf("ndshell: %s: command not found. Process %d has been terminated.\n", command[0], getpid());
        exit(1);
    }
    else {
        int cpid = rc;
        runCPID = cpid;
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

    numChildProcesses--;
}

void nd_wait(int pid) {
    int wstatus;
    int cpid = waitpid(pid, &wstatus, 0);
    if (WTERMSIG(wstatus)) {
        printf("Process %d exited abnormally with signal %d\n", cpid, WTERMSIG(wstatus));
    }
    else {
        printf("Process %d exited normally with status %d\n", cpid, WEXITSTATUS(wstatus));
    }
    fflush(stdout);

    removeChildProcess(cpid);
}

void signalCapture(int signum) {
    if (numChildProcesses == 0) {
            exit(1);
    }
    int cpid = childProcesses[numChildProcesses - 1];
    if (signum == SIGINT) {
        if (!isChildProccess(getpid())) {
            nd_wait(cpid);
        }
    }
}

int main(int argc, char* argv[]) {

    if (argc != 1) {
        fprintf(stderr, "ndshell: Executable does not take any parameters!\n");
        exit(1);
    }

    struct sigaction sigSetValue;
    sigSetValue.sa_handler = signalCapture;
    sigaction(SIGINT, &sigSetValue, NULL);
    sigaction(SIGKILL, &sigSetValue, NULL);

    const int ppid = getpid();

    int flag = 1;
    while (flag && ppid == getpid()) {

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

        if (strcmp(command[0], "exit") == 0) {
            flag = 0;
        }
        else if (strcmp(command[0], "start") == 0) {
            // call fork and exec and keep track of cpid
            if (counter <= 1) {
                fprintf(stderr, "ndshell: Invalid start command! Please enter a process to run.\n");
                continue;
            }

            createChildProcess(executable);
        }
        else if (strcmp(command[0], "wait") == 0) {
            // wait for any child process to exit
            if (numChildProcesses == 0) {
                printf("ndshell: There are currently no processes running.\n");
            }
            else {
                nd_wait(-1);
            }
        }
        else if (strcmp(command[0], "waitfor") == 0) {
            // make sure counter is greater than 1
            // get the specified cpid
            // wait for specific cpid
            if (counter <= 1) {
                fprintf(stderr, "ndshell: Invalid waitfor command! Invalid pid specified.\n");
                continue;
            }
            int cpid = atoi(command[1]);
            
            if (numChildProcesses == 0) {
                printf("ndshell: There are currently no processes running.\n");
            }
            else if (!isChildProccess(cpid)) {
                fprintf(stderr, "ndshell: Process %d does not exist!\n", cpid);
            }
            else {
                nd_wait(cpid);
            }
        }
        else if (strcmp(command[0], "run") == 0) {
            // run start and waitfor for given process
            if (counter <= 1) {
                fprintf(stderr, "ndshell: Invalid start command! Please enter a process to run.\n");
                continue;
            }
            createChildProcess(executable);
            nd_wait(runCPID);

        }
        else if (strcmp(command[0], "kill") == 0) {
            // make sure counter is greater than 1
            // get the specified cpid
            // kill specific cpid
            if (counter <= 1) {
                fprintf(stderr, "ndshell: Invalid kill command! Please enter a process id to kill.\n");
                continue;
            }
            int cpid = atoi(command[1]);
            
            if (numChildProcesses == 0) {
                printf("ndshell: There are currently no processes running.\n");
            }
            else if (!isChildProccess(cpid)) {
                fprintf(stderr, "ndshell: Process %d does not exist!\n", cpid);
            }
            else {
                kill(cpid, SIGKILL);
                nd_wait(cpid);
            }

        }
        else if (strcmp(command[0], "quit") == 0) {
            // wait for all child processes
            // exit gracefully
            int copyCPID[numChildProcesses];
            int copyNUM = numChildProcesses;
            int i;
            for (i = 0; i < numChildProcesses; i++) {
                copyCPID[i] = childProcesses[i];
            }

            for (i = 0; i < copyNUM; i++) {
                kill(copyCPID[i], SIGKILL);
                nd_wait(copyCPID[i]);
            }

            printf("\nndshell: All child processes complete - exiting the shell.\n");
            break;

        }
        else if (strcmp(command[0], "bound") == 0) {
            if (counter <= 2) {
                fprintf(stderr, "ndshell: Invalid bound command! Please enter number of seconds and a command.\n");
                continue;
            }
            // check params
            int sec = atoi(command[1]);
            if (sec == 0) {
                fprintf(stderr, "ndshell: Invalid bound command! Please enter a valid number of seconds for the command to run.\n");
                continue;
            }
            char* bound_executable[maxCommandLen - 2];
            int i;
            for (i = 2; i <= counter; i++) {
                bound_executable[i-2] = command[i];
            }

            // run and wait for given process
            createChildProcess(bound_executable);
            int cpid = runCPID;

            int rc = fork();
            if (rc < 0) {
                fprintf(stderr, "Fork Failed\n");
                exit(1);
            }
            else if (rc == 0) {
                sleep(sec);
                kill(cpid, SIGKILL);
            }
            else {
                nd_wait(runCPID);
                wait(NULL); // wait for timer to return
            }

        }
        else {
            printf("ndshell: Unknown command %s\n", command[0]);
        }

    }
    return 0;
}