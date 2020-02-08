#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "config.h"

#define OUTFILE "mandelbrot_para_write.out"

typedef struct coordinates
{
	double x;
	double y;
	unsigned iter;
} coordinates;

typedef coordinates * p_coordinates;

p_coordinates createCoordinates( double x, double y, unsigned iter )
{
	p_coordinates p_coord = ( p_coordinates )malloc( sizeof( p_coordinates ) );
	p_coord->x = x;
	p_coord->y = y;
	p_coord->iter = iter;
	return p_coord;
}

// Type definition
typedef struct WriterStack {
	int size;
	unsigned capacity;
	unsigned tokens;
	FILE * file;
	p_coordinates * stack;
} WriterStack;

typedef WriterStack * p_writerStack;

// Create a writerStack
p_writerStack createWriterStack( unsigned capacity, FILE *file, unsigned expiration ) 
{ 
    p_writerStack ws = ( p_writerStack )malloc( sizeof( p_writerStack ) );
    if ( file == NULL ) {
    	return;
    }
    ws->file = file;
    ws->capacity = capacity; 
    ws->size = 0;
    ws->stack = ( p_coordinates* )malloc( ws->capacity * sizeof( p_coordinates ) );
    ws->tokens = expiration;
    return ws;
}

// Is WriterStack full ?
int isFull(p_writerStack ws) 
{ 
    return ws->size == ws->capacity; 
} 

// Is WriterStack empty ?
int isEmpty(p_writerStack ws) 
{ 
    return ws->size == 0; 
}

int isAvailable(p_writerStack ws)
{
	return ws->tokens > 0;
}

p_coordinates top(p_writerStack ws) 
{ 
    if (isEmpty(ws)){
        return;
    }
    return ws->stack[ws->size-1]; 
} 

void pop(p_writerStack ws) 
{ 
    if (isEmpty(ws)){
        return;
    }
  	p_coordinates coord = top(ws);
    fprintf(ws->file, "%f %f %d\n", coord->x, coord->y, coord->iter);
    ws->stack[--ws->size] = NULL;
}

void push(p_writerStack ws, p_coordinates item) 
{ 
    if (isFull(ws) || isAvailable(ws) == 0){
        return;
    }
    ws->stack[++ws->size] = item;
    if ( --ws->tokens == 0 )
    {
    	fprintf( ws->file, "\n" );
    	fclose( ws->file );
    }
    pop(ws);

}

struct threadArguments
{
	int threadNum;
	int start;
	int end;
	p_writerStack writerStack;
	double * threadTimes;
};

void * threadFunction(void * threadArgs)
{
	struct threadArguments *args = (struct threadArguments *)threadArgs;
	
	p_writerStack ws = args->writerStack;

	time_t timerStart = time(NULL);

	for (int xpixel = args->start; xpixel < args->end; xpixel++ )
	{
		for (int ypixel = 0; ypixel < nbpixely; ypixel++ )
		{
			double xinit = XMIN + xpixel * RESOLUTION;
			double yinit = YMIN + ypixel * RESOLUTION;

			double x = xinit;
			double y = yinit;

			int iter = 0;
			for ( iter = 0; iter < NITERMAX; iter++ )
			{
				double prevy = y;
				double prevx = x;

				if ((x * x + y * y) > 4)
				{
					break;
				}
				x = prevx * prevx - prevy * prevy + xinit;
				y = 2 * prevx * prevy + yinit;
			}

			x = XMIN + xpixel * RESOLUTION;
			y = YMIN + ypixel * RESOLUTION;

			push( ws, createCoordinates( xpixel, ypixel, iter ) );
		}
	}

	time_t timerEnd = time(NULL);

	args->threadTimes[args->threadNum] = difftime( timerEnd, timerStart );

	pthread_exit( NULL );
}

int main(int argc, char **argv)
{
	double averageThreadTime, totalTime;
	double * threadTimes;
	int offset, rest, thread_id;
	time_t timerStart, timerEnd;
	pthread_t thread[NBTHREAD];
	struct threadArguments threadArgs[NBTHREAD];
	p_writerStack writerStack;

	nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
	nbpixely = ceil((YMAX - YMIN) / RESOLUTION);

	writerStack = createWriterStack( nbpixelx, fopen(OUTFILE, "w"), nbpixelx);

	offset = nbpixelx / NBTHREAD;
	rest = nbpixelx - NBTHREAD * offset;

	printf("\nThreads ... : %d\nPixels .... : %d\nOffset .... : %d\nRest ...... : %d\n", NBTHREAD, nbpixelx, offset, rest );

	if ((threadTimes = malloc( sizeof(double) * NBTHREAD ) ) == NULL) {
		printf("ERREUR d'allocation de threadtimes[], errno : %d (%s) .\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	timerStart = time(NULL);

	for ( thread_id = 0; thread_id < NBTHREAD; thread_id++ )
	{
		int start = thread_id == 0 ? 0 : threadArgs[thread_id - 1].end;
		int end = start + offset;

		if ( rest-- > 0 ) {
			end = end > nbpixelx ? nbpixelx : end + 1;
		}

		threadArgs[thread_id].start = start;
		threadArgs[thread_id].end = end;
		threadArgs[thread_id].threadNum = thread_id;
		threadArgs[thread_id].writerStack = writerStack;
		threadArgs[thread_id].threadTimes = threadTimes;

		if (pthread_create(&thread[thread_id], NULL, threadFunction, (void *)&threadArgs[thread_id]) != 0)
		{
			fprintf(stderr, "Erreur à la création du thread %d\n", thread_id);
			return EXIT_FAILURE;
		}
	}
	/*
	 * Attente de la fin des threads précédemment lancées
	 */
	for ( thread_id = 0; thread_id < NBTHREAD; thread_id++ )
	{
		void * ret_ptr;
		pthread_join(thread[thread_id], &ret_ptr);
	}

	timerEnd = time(NULL);

	totalTime = difftime( timerEnd, timerStart );
	averageThreadTime = 0;
	for ( size_t i = 0 ; i < NBTHREAD ; i++ ) {
		averageThreadTime += threadTimes[i];
	}

	printf("Total Time  : %.4f seconds\n", totalTime );
	printf("Thread Time : %.4f seconds ( average )\n", averageThreadTime / NBTHREAD );

	/* Clean */
	free(writerStack);

	/*sortie du programme*/
	return EXIT_SUCCESS;
}
