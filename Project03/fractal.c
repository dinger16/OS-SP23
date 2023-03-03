/*
fractal.c - Mandelbrot fractal generation
Starting code for CSE 30341 Project 3 - Spring 2023
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>

#include "bitmap.h"
#include "fractal.h"

/*
Compute the number of iterations at point x, y
in the complex space, up to a maximum of maxiter.
Return the number of iterations at that point.

This example computes the Mandelbrot fractal:
z = z^2 + alpha

Where z is initially zero, and alpha is the location x + iy
in the complex plane.  Note that we are using the "complex"
numeric type in C, which has the special functions cabs()
and cpow() to compute the absolute values and powers of
complex values.
*/
#define MAX_THREADS 40

#define TASK_HEIGHT 20
#define TASK_WIDTH 20

pthread_mutex_t lock;

struct Task {
    int startX;
    int endX;
    int startY;
    int endY;
    int active;
};

struct ThreadInfo {
    int         nIndex;
    pthread_t   threadId;
    int         taskCount;
    struct      Task *task;
    struct      FractalSettings *settings;
    struct      bitmap *map;
};

struct ThreadInfo TheThreads[MAX_THREADS];

static int compute_point( double x, double y, int max )
{
	double complex z = 0;
	double complex alpha = x + I*y;

	int iter = 0;

	while( cabs(z)<4 && iter < max ) {
		z = cpow(z,2) + alpha;
		iter++;
	}

	return iter;
}

/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).

HINT: Generally, you will want to leave this code alone and write your threaded code separately

*/

void compute_image_singlethread ( struct FractalSettings * pSettings, struct bitmap * pBitmap)
{
	int i,j;

	// For every pixel i,j, in the image...

	for(j=0; j<pSettings->nPixelHeight; j++) {
		for(i=0; i<pSettings->nPixelWidth; i++) {

			// Scale from pixels i,j to coordinates x,y
			double x = pSettings->fMinX + i*(pSettings->fMaxX - pSettings->fMinX) / pSettings->nPixelWidth;
			double y = pSettings->fMinY + j*(pSettings->fMaxY - pSettings->fMinY) / pSettings->nPixelHeight;

			// Compute the iterations at x,y
			int iter = compute_point(x,y,pSettings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / pSettings->nMaxIter;

            // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pBitmap,i,j,gray);
		}
	}
}

void * compute_image_multithread (void * pData)
{
    struct ThreadInfo *pThreadInfo;

    pThreadInfo = (struct ThreadInfo *) pData;

    int index = pThreadInfo->nIndex;
    int numThreads = pThreadInfo->settings->nThreads;
    int height = pThreadInfo->settings->nPixelHeight;

    int start = index * (height / numThreads);
    int stop = ((index+1) * (height / numThreads) >= height) ? height : (index+1) * (height / numThreads);

	int i,j;
	for(j=start; j<stop; j++) {
		for(i=0; i<pThreadInfo->settings->nPixelWidth; i++) {

			double x = pThreadInfo->settings->fMinX + i*(pThreadInfo->settings->fMaxX - pThreadInfo->settings->fMinX) / pThreadInfo->settings->nPixelWidth;
			double y = pThreadInfo->settings->fMinY + j*(pThreadInfo->settings->fMaxY - pThreadInfo->settings->fMinY) / pThreadInfo->settings->nPixelHeight;
			// Compute the iterations at x,y
			int iter = compute_point(x,y,pThreadInfo->settings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter * 4 / pThreadInfo->settings->nMaxIter;

            // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pThreadInfo->map,i,j,gray);
		}
	}
    return NULL;
}


void * compute_image_tasks (void * pData)
{
    pthread_mutex_lock(&lock);

    struct ThreadInfo *pThreadInfo;

    pThreadInfo = (struct ThreadInfo *) pData;

    int index = 0;
    for (int i = 0; i < pThreadInfo->taskCount; i++) {
        if (pThreadInfo->task[i].active == 1) {
            index = i;
            break;
        }
    }

    int startX = pThreadInfo->task[index].startX;
    int endX = pThreadInfo->task[index].endX;
    int startY = pThreadInfo->task[index].startY;
    int endY = pThreadInfo->task[index].endY;

	int i,j;
	for(j=startY; j<endY; j++) {
		for(i=startX; i<endX; i++) {

			double x = pThreadInfo->settings->fMinX + i*(pThreadInfo->settings->fMaxX - pThreadInfo->settings->fMinX) / pThreadInfo->settings->nPixelWidth;
			double y = pThreadInfo->settings->fMinY + j*(pThreadInfo->settings->fMaxY - pThreadInfo->settings->fMinY) / pThreadInfo->settings->nPixelHeight;
			// Compute the iterations at x,y
			int iter = compute_point(x,y,pThreadInfo->settings->nMaxIter);

			// Convert a iteration number to an RGB color.
			// (Change this bit to get more interesting colors.)
			int gray = 255 * iter / pThreadInfo->settings->nMaxIter;

            // Set the particular pixel to the specific value
			// Set the pixel in the bitmap.
			bitmap_set(pThreadInfo->map,i,j,gray);
		}
	}
    pThreadInfo->task[index].active = 0;

    pthread_mutex_unlock(&lock);

    return NULL;
}


/* Process all of the arguments as provided as an input and appropriately modify the
   settings for the project 
   @returns 1 if successful, 0 if unsuccessful (bad arguments) */
char processArguments (int argc, char * argv[], struct FractalSettings * pSettings)
{

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-help") == 0) {
            fprintf(stderr, "Flags:\n");
            fprintf(stderr, "  -help: Print this message\n");
            fprintf(stderr, "  -width <value>: Set the width of the image in pixels\n");
            fprintf(stderr, "  -height <value>: Set the height of the image in pixels\n");
            fprintf(stderr, "  -maxiter <value>: Set the maximum number of iterations\n");
            fprintf(stderr, "  -xmin <value>: Set the minimum x value\n");
            fprintf(stderr, "  -xmax <value>: Set the maximum x value\n");
            fprintf(stderr, "  -ymin <value>: Set the minimum y value\n");
            fprintf(stderr, "  -ymax <value>: Set the maximum y value\n");
            fprintf(stderr, "  -threads: Set the number of threads to use\n");
            fprintf(stderr, "  -row: Set parallelization by row\n");
            fprintf(stderr, "  -task: Set parallelization by task\n");
            fprintf(stderr, "  -output <filename>: Set the output file name\n");
            exit(0);
        } else if (strcmp(argv[i], "-xmin") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -xmin requires a value\n");
                exit(1);
            } else {
                float new_value = atof(argv[i]);
                if (new_value == 0 && strcmp(argv[i], "0") != 0) {
                    fprintf(stderr, "Error: -xmin requires a numeric value\n");
                    exit(1);
                } else {
                    pSettings->fMinX = new_value;
                }
            }
        } else if (strcmp(argv[i], "-xmax") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -xmax requires a value\n");
                exit(1);
            } else {
                float new_value = atof(argv[i]);
                if (new_value == 0 && strcmp(argv[i], "0") != 0) {
                    fprintf(stderr, "Error: -xmax requires a numeric value\n");
                    exit(1);
                } else {
                    pSettings->fMaxX = new_value;
                }
            }
        } else if (strcmp(argv[i], "-ymin") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -ymin requires a value\n");
                exit(1);
            } else {
                float new_value = atof(argv[i]);
                if (new_value == 0 && strcmp(argv[i], "0") != 0) {
                    fprintf(stderr, "Error: -ymin requires a numeric value\n");
                    exit(1);
                } else {
                    pSettings->fMinY = new_value;
                }
            }
        } else if (strcmp(argv[i], "-ymax") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -ymax requires a value\n");
                exit(1);
            } else {
                float new_value = atof(argv[i]);
                if (new_value == 0 && strcmp(argv[i], "0") != 0) {
                    fprintf(stderr, "Error: -ymax requires a numeric value\n");
                    exit(1);
                } else {
                    pSettings->fMaxY = new_value;
                }
            }
        } else if (strcmp(argv[i], "-maxiter") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -maxiter requires a value\n");
                exit(1);
            } else {
                int new_value = atoi(argv[i]);
                if (new_value <= 0) {
                    fprintf(stderr, "Error: -maxiter requires a positive integer value\n");
                    exit(1);
                } else {
                    pSettings->nMaxIter = new_value;
                }
            }
        } else if (strcmp(argv[i], "-width") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -width requires a value\n");
                exit(1);
            } else {
                int new_value = atoi(argv[i]);
                if (new_value <= 0) {
                    fprintf(stderr, "Error: -width requires a positive value\n");
                    exit(1);
                } else {
                    pSettings->nPixelWidth = new_value;
                }
            }
        } else if (strcmp(argv[i], "-height") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -height requires a value\n");
                exit(1);
            } else {
                int new_value = atoi(argv[i]);
                if (new_value <= 0) {
                    fprintf(stderr, "Error: -height requires a positive value\n");
                    exit(1);
                } else {
                    pSettings->nPixelHeight = new_value;
                }
            }
        } else if (strcmp(argv[i], "-output") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -output requires a value\n");
                exit(1);
            } else {
                char* new_value = argv[i];
                if (new_value == NULL) {
                    fprintf(stderr, "Error: -output requires a valid file name\n");
                    exit(1);
                } else {
                    strcpy(pSettings->szOutfile, new_value);
                }
            }
        } else if (strcmp(argv[i], "-threads") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Error: -threads requires a value\n");
                exit(1);
            } else {
                int new_value = atoi(argv[i]);
                if (new_value <= 0 || new_value > MAX_THREADS) {
                    fprintf(stderr, "Error: -threads requires a positive integer value that is less than %d\n", MAX_THREADS);
                    exit(1);
                } else {
                    pSettings->nThreads = new_value;
                }
            }
        } else if (strcmp(argv[i], "-row") == 0) {
            // decide to run with row based approach
            pSettings->theMode = MODE_THREAD_ROW;
        } else if (strcmp(argv[i], "-task") == 0) {
            // decide to run with thread based approach
            pSettings->theMode = MODE_THREAD_TASK;
        } else {
            fprintf(stderr, "Error: invalid argument %s\n", argv[i]);
            exit(1);
        }

        i++;
    }

    /* If we don't process anything, it must be successful, right? */
    return 1;
}


int main( int argc, char *argv[] )
{
    struct FractalSettings  theSettings;

	// The initial boundaries of the fractal image in x,y space.
    theSettings.fMinX = DEFAULT_MIN_X;
    theSettings.fMaxX = DEFAULT_MAX_X;
    theSettings.fMinY = DEFAULT_MIN_Y;
    theSettings.fMaxY = DEFAULT_MAX_Y;
    theSettings.nMaxIter = DEFAULT_MAX_ITER;

    theSettings.nPixelWidth = DEFAULT_PIXEL_WIDTH;
    theSettings.nPixelHeight = DEFAULT_PIXEL_HEIGHT;

    theSettings.nThreads = DEFAULT_THREADS;
    theSettings.theMode  = MODE_THREAD_SINGLE;
    
    strncpy(theSettings.szOutfile, DEFAULT_OUTPUT_FILE, MAX_OUTFILE_NAME_LEN);

    /* TODO: Adapt your code to use arguments where the arguments can be used to override 
             the default values 

        -help         Display the help information
        ----------------
        -xmin X       New value for x min
        -xmax X       New value for x max
        -ymin Y       New value for y min
        -ymax Y       New value for y max
        -maxiter N    New value for the maximum number of iterations (must be an integer)     
        -width W      New width for the output image
        -height H     New height for the output image
        ----------------
        -output F     New name for the output file
        -threads N    Number of threads to use for processing (default is 1) 
        -row          Run using a row-based approach        
        -task         Run using a thread-based approach

        Support for setting the number of threads is optional

        You may also appropriately apply reasonable minimum / maximum values (e.g. minimum image width, etc.)
    */


   /* Are there any locks to set up? */


   if(processArguments(argc, argv, &theSettings))
   {
        /* Dispatch here based on what mode we might be in */
        if(theSettings.theMode == MODE_THREAD_SINGLE)
        {
            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Compute the image */
            compute_image_singlethread(&theSettings, pBitmap);

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }            
        }
        else if(theSettings.theMode == MODE_THREAD_ROW)
        {
            /* A row-based approach will not require any concurrency protection */

            /* Could you send an argument and write a different version of compute_image that works off of a
               certain parameter setting for the rows to iterate upon? */
            
            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Create the threads */
            int i;
            for (i = 0; i < theSettings.nThreads; i++) {
                TheThreads[i].nIndex = i;
                TheThreads[i].settings = &theSettings;
                TheThreads[i].map = pBitmap;
                pthread_create(&TheThreads[i].threadId, NULL, compute_image_multithread, &TheThreads[i]);
            }

            /* Join the threads */
            for (i = 0; i < theSettings.nThreads; i++) {
                pthread_join(TheThreads[i].threadId, NULL);
            }

            // Save the image in the stated file.
            if(!bitmap_save(pBitmap,theSettings.szOutfile)) {
                fprintf(stderr,"fractal: couldn't write to %s: %s\n",theSettings.szOutfile,strerror(errno));
                return 1;
            }    
        }
        else if(theSettings.theMode == MODE_THREAD_TASK)
        {
            /* For the task-based model, you will want to create some sort of a way that captures the instructions
               or task (perhaps say a startX, startY and stopX, stopY in a struct).  You can have a global array 
               of the particular tasks with each thread attempting to pop off the next task.  Feel free to tinker 
               on what the right size of the work unit is but 20x20 is a good starting point.  You are also welcome
               to modify the settings struct to help you out as well.  
               
               Generally, it will be good to create all of the tasks into that array and then to start your threads
               with them in turn attempting to pull off a task one at a time.  
               
               While we could do condition variables, there is not really an ongoing producer if we create all of
               the tasks at the outset. Hence, it is OK whenever a thread needs something to do to try to access
               that shared data structure with all of the respective tasks.  
               */

            int height = theSettings.nPixelHeight;
            int width = theSettings.nPixelWidth;
            int curHeight = height;
            int curWidth = width;

            int nTasks = (height / TASK_HEIGHT + 1) * (width / TASK_WIDTH + 1);
            struct Task TheTasks[nTasks - 1];

            int taskCount = 0;
            while (curHeight >= TASK_HEIGHT) {
                curWidth = width;
                while (curWidth >= TASK_WIDTH) {
                    TheTasks[taskCount].startX = width - curWidth;
                    TheTasks[taskCount].startY = height - curHeight;
                    TheTasks[taskCount].endX = width - curWidth + TASK_WIDTH;
                    TheTasks[taskCount].endY = height - curHeight + TASK_HEIGHT;
                    TheTasks[taskCount].active = 1;
                    taskCount ++;
                    curWidth -= TASK_WIDTH;
                }
                if (curWidth > TASK_WIDTH) {
                    TheTasks[taskCount].startX = width - curWidth;
                    TheTasks[taskCount].startY = height - curHeight;
                    TheTasks[taskCount].endX = width;
                    TheTasks[taskCount].endY = height - curHeight + TASK_HEIGHT;
                    TheTasks[taskCount].active = 1;
                    taskCount ++;
                }
                curHeight -= TASK_HEIGHT;
            }
            curWidth = width;
            if (curHeight > 0) {
                while (curWidth >= TASK_WIDTH) {
                    TheTasks[taskCount].startX = width - curWidth;
                    TheTasks[taskCount].startY = height - curHeight;
                    TheTasks[taskCount].endX = width - curWidth + TASK_WIDTH;
                    TheTasks[taskCount].endY = height;
                    TheTasks[taskCount].active = 1;
                    taskCount ++;
                    curWidth -= TASK_WIDTH;
                }
                if (curWidth > TASK_WIDTH) {
                    TheTasks[taskCount].startX = width - curWidth;
                    TheTasks[taskCount].startY = height - curHeight;
                    TheTasks[taskCount].endX = width;
                    TheTasks[taskCount].endY = height;
                    TheTasks[taskCount].active = 1;
                    taskCount ++;
                }
            }
            
            /* Create a bitmap of the appropriate size */
            struct bitmap * pBitmap = bitmap_create(theSettings.nPixelWidth, theSettings.nPixelHeight);

            /* Fill the bitmap with dark blue */
            bitmap_reset(pBitmap,MAKE_RGBA(0,0,255,0));

            /* Create the threads */
            int i;
            for (i = 0; i < theSettings.nThreads; i++) {
                TheThreads[i].nIndex = i;
                TheThreads[i].taskCount = taskCount;
                TheThreads[i].task = (struct Task*) TheTasks;
                TheThreads[i].settings = &theSettings;
                TheThreads[i].map = pBitmap;
                //pthread_create(&TheThreads[i].threadId, NULL, compute_image_multithread, &TheThreads[i]);
                pthread_create(&TheThreads[i].threadId, NULL, compute_image_tasks, &TheThreads[i]);
            }

            /* Join the threads */
            for (i = 0; i < theSettings.nThreads; i++) {
                pthread_join(TheThreads[i].threadId, NULL);
            }

        }
        else 
        {
            /* Uh oh - how did we get here? */
        }
   }
   else
   {
        /* Probably a great place to dump the help */

        /* Probably a good place to bail out */
        exit(-1);
   }

    /* TODO: Do any cleanup as required */

	return 0;
}
