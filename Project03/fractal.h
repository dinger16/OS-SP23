/* Fractal.h : Definitions and default values for fractal */

#ifndef __FRACTAL_H
#define __FRACTAL_H

#include "bitmap.h"

/* Default values for the fractal ranges and settings */
#define DEFAULT_MIN_X        -1.5
#define DEFAULT_MAX_X        0.5
#define DEFAULT_MIN_Y        -1.0
#define DEFAULT_MAX_Y        1.0
#define DEFAULT_MAX_ITER    500

/* Default image sizes */
#define DEFAULT_PIXEL_WIDTH     640
#define DEFAULT_PIXEL_HEIGHT    480

#define DEFAULT_OUTPUT_FILE     "fractal-out.bmp"
#define MAX_OUTFILE_NAME_LEN    32

/* Default thread settings (if row or task is enabled) */
#define DEFAULT_THREADS 2

enum ComputeMode 
{
    MODE_THREAD_SINGLE,
    MODE_THREAD_ROW,
    MODE_THREAD_TASK
};


struct FractalSettings 
{
    double  fMinX;
    double  fMaxX;
    double  fMinY;
    double  fMaxY;
    int     nMaxIter;

    /* Size of the image to generate */
    int     nPixelWidth;
    int     nPixelHeight;

    char    szOutfile[MAX_OUTFILE_NAME_LEN+1];

    /* Mode with regards to computation */
    enum ComputeMode     theMode;
    int                  nThreads;
};




/* Function prototypes */

static int compute_point( double x, double y, int max );

void compute_image_singlethread ( struct FractalSettings * pSettings, struct bitmap * pBitmap);

#endif
