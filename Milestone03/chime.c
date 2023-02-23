#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

char g_bKeepLooping = 1;
#define MAX_THREADS 5
#define BUFFER_SIZE 1024
#define maxCommandLen 10
pthread_mutex_t lock;

struct ChimeThreadInfo {
    int        nIndex;
    float      fChimeInterval;
    char       bIsValid;
    pthread_t  ThreadID;
};

struct ChimeThreadInfo TheThreads[MAX_THREADS];

void * ThreadChime (void * pData) {
    struct ChimeThreadInfo  * pThreadInfo;

    pThreadInfo = (struct ChimeThreadInfo *) pData;
    while(g_bKeepLooping) {
        // lock the mutex for sleep and the print statement
        pthread_mutex_lock(&lock);
        sleep(pThreadInfo->fChimeInterval);
        printf("Ding - Chime %d with an interval of %f s!\n", pThreadInfo->nIndex, pThreadInfo->fChimeInterval);
        // unlock the mutex
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main (int argc, char *argv[]) {

    char szBuffer[BUFFER_SIZE];
    /* Set all of the thread information to be invalid (none allocated) */
    for (int j=0; j<MAX_THREADS; j++) {
        TheThreads[j].bIsValid = 0;
    }
    
    while(1) {
        /* Prompt and flush to stdout */
        printf("CHIME>");
        fflush(stdout);
        /* Wait for user input via fgets */
        char* fgets_return = fgets(szBuffer, BUFFER_SIZE, stdin);
        
        if (fgets_return == NULL) {
            break;
        }

        // remove the newline character and replace with null terminator and split the string
        szBuffer[strlen(szBuffer) - 1] = '\0';
        char* split = strtok(szBuffer, " ");
        char* command[maxCommandLen];

        // parse the command and arguments and count how many there are
        int counter = 0;
        while (split != NULL && counter < maxCommandLen) {
            command[counter] = split;
            split = strtok(NULL, " ");
            counter++;
        }
        // set the last element to null
        command[counter] = NULL;

        // if user clicked enter then continue
        if (counter == 0) {
            continue;
        } else if (strcmp(command[0], "chime") == 0) {
            // call chime
            // if user didn't use 3 arguments (chime, index, interval) then exit
            if (counter != 3) {
                printf("CHIME: chime command requires two arguments\n");
                continue;
            }

            // convert index to integer
            int index = atoi(command[1]);
            // check if index is valid (atoi returns 0 if it can't convert)
            if (index == 0 && strcmp(command[1], "0") != 0) {
                printf("CHIME: chime index must be an integer\n");
                continue;
            // check if index is in range
            } else if (index < 0 || index >= MAX_THREADS) {
                printf("CHIME: cannot adjust chime %d, out of range\n", index);
                continue;
            }

            // convert interval to float
            int seconds = atof(command[2]);
            // check if interval is valid (atof returns 0 if it can't convert)
            if (seconds == 0 && strcmp(command[2], "0") != 0) {
                printf("CHIME: chime interval must be a floating point number\n");
                continue;
            // check if interval is greater than 0
            } else if (seconds <= 0) {
                printf("CHIME: chime interval must be greater than 0\n");
                continue;
            }

            // if thread is not valid then create it
            if (TheThreads[index].bIsValid == 0) {
                TheThreads[index].nIndex = index;
                TheThreads[index].fChimeInterval = seconds;
                TheThreads[index].bIsValid = 1;
                pthread_create(&TheThreads[index].ThreadID, NULL, ThreadChime, &TheThreads[index]);
                printf("Starting thread %ld for chime %d, interval of %d s\n", TheThreads[index].ThreadID, index, seconds);
            // if thread is valid then just change the interval
            } else {
                TheThreads[index].fChimeInterval = seconds;
                printf("Adjusting chime %d to interval of %d s\n", index, seconds);
            }

        } else if (strcmp(command[0], "exit") == 0) {
            // call exit
            // if user gave more arguments than they should have then exit
            if (counter > 1) {
                printf("CHIME: exit command has no additional arguments\n");
            }
            // stop looping and join all threads
            g_bKeepLooping = 0;
            for (int i=0; i<MAX_THREADS; i++) {
                if (TheThreads[i].bIsValid == 1) {
                    printf("Joining chime %d (Thread %ld)\n", i, TheThreads[i].ThreadID);
                    pthread_join(TheThreads[i].ThreadID, NULL);
                    printf("Join complete for chime %d\n", i);
                }
            }
            printf("Exiting chime program...\n");
            break;
        // print message if user entered an invalid command
        } else {
            printf("CHIME: Unknown command %s\n", command[0]);
        }    
    }

    return 0;
}