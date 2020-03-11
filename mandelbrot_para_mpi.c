#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "mpi.h"
#include "config.h"

#define OUTFILE "mandelbrot_para.out"

long double getMicrotime()
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

struct threadArguments
{
	int threadNum;
	int start;
	int end;
	int * itertab;
	long double * threadTimes;
};

void * threadFunction(void * threadArgs)
{
	struct threadArguments *args = (struct threadArguments *)threadArgs;
	
	int * itertab = args->itertab;

	long double timerStart = getMicrotime();

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

			itertab[xpixel * nbpixely + ypixel] = iter;
		}
	}

	long double timerEnd = getMicrotime();

	args->threadTimes[args->threadNum] = (timerEnd - timerStart) / 1e6;

	pthread_exit( NULL );
}

int main(int argc, char **argv)
{
	MPI_Init(&argc,&argv);
	//long double averageThreadTime;
	//long double * threadTimes;
	//int offset, rest, thread_id;
	//int * itertab;
	//FILE *file;
	//long double totalTime, timerStart, timerEnd;
	//pthread_t thread[NBTHREAD];
	//struct threadArguments threadArgs[NBTHREAD];
//
	//nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
	//nbpixely = ceil((YMAX - YMIN) / RESOLUTION);
//
	//offset = nbpixelx / NBTHREAD;
	//rest = nbpixelx - NBTHREAD * offset;
//
	//printf("\nThreads ... : %d\nPixels .... : %d\nOffset .... : %d\nRest ...... : %d\n", NBTHREAD, nbpixelx, offset, rest );
//
	///*
	// * Allocation du tableau de pixel
	// */
	//if ((itertab = malloc(sizeof(int) * nbpixelx * nbpixely)) == NULL)
	//{
	//	printf("ERREUR d'allocation de itertab[], errno : %d (%s) .\n", errno, strerror(errno));
	//	return EXIT_FAILURE;
	//}
	//if ((threadTimes = malloc( sizeof(long double) * NBTHREAD ) ) == NULL) {
	//	printf("ERREUR d'allocation de threadtimes[], errno : %d (%s) .\n", errno, strerror(errno));
	//	return EXIT_FAILURE;
	//}
//
	//timerStart = getMicrotime();
//
	//for ( thread_id = 0; thread_id < NBTHREAD; thread_id++ )
	//{
	//	int start = thread_id == 0 ? 0 : threadArgs[thread_id - 1].end;
	//	int end = start + offset;
//
	//	if ( rest-- > 0 ) {
	//		end = end > nbpixelx ? nbpixelx : end + 1;
	//	}
//
	//	threadArgs[thread_id].start = start;
	//	threadArgs[thread_id].end = end;
	//	threadArgs[thread_id].threadNum = thread_id;
	//	threadArgs[thread_id].itertab = itertab;
	//	threadArgs[thread_id].threadTimes = threadTimes;
//
	//	if (pthread_create(&thread[thread_id], NULL, threadFunction, (void *)&threadArgs[thread_id]) != 0)
	//	{
	//		fprintf(stderr, "Erreur à la création du thread %d\n", thread_id);
	//		return EXIT_FAILURE;
	//	}
	//}
	///*
	// * Attente de la fin des threads précédemment lancées
	// */
	//for ( thread_id = 0; thread_id < NBTHREAD; thread_id++ )
	//{
	//	void * ret_ptr;
	//	pthread_join(thread[thread_id], &ret_ptr);
	//}
//
	//timerEnd = getMicrotime();
//
	//totalTime = (timerEnd - timerStart) / 1e6;
	//averageThreadTime = 0;
	//for ( size_t i = 0 ; i < NBTHREAD ; i++ ) {
	//	averageThreadTime += threadTimes[i];
	//}
//
	//printf("Total Time  : %.6Lf seconds\n", totalTime );
	//printf("Thread Time : %.6Lf seconds ( average )\n", averageThreadTime / NBTHREAD );
//
	//if ( WRITE ) {
//
	//	/*
	// * Output des resultats compatible GNUPLOT
	// */
	//	if ((file = fopen(OUTFILE, "w")) == NULL)
	//	{
	//		printf("Erreur à l'ouverture du fichier de sortie : errno %d (%s) .\n", errno, strerror(errno));
	//		return EXIT_FAILURE;
	//	}
//
	//	for (int xpixel = 0; xpixel < nbpixelx; xpixel++)
	//	{
	//		for (int ypixel = 0; ypixel < nbpixely; ypixel++)
	//		{
	//			double x = XMIN + xpixel * RESOLUTION;
	//			double y = YMIN + ypixel * RESOLUTION;
	//			fprintf(file, "%f %f %d\n", x, y, itertab[xpixel * nbpixely + ypixel]);
	//		}
	//		fprintf(file, "\n");
	//	}
	//	fclose(file);
//
	//}
//
	///* Clean */
	//free(itertab);
//
	///*sortie du programme*/
	MPI_Finalize();
	return EXIT_SUCCESS;
}
