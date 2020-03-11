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

long double getMicrotime()
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

//TODO: il faudra changer le nom de ça
struct threadArguments
{
	int threadNum;
	int start;
	int end;
	int * itertab;
	long double * threadTimes;
};

//TODO: il faudra changer le nom de ça
void threadFunction(void * threadArgs)
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
}

void exitWithCode (const int exitCode)
{
	MPI_Finalize();
	exit(exitCode);
}

int main(int argc, char **argv)
{
	int NBPROGRAMS, realRank, displayRank;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&realRank);
	MPI_Comm_size(MPI_COMM_WORLD,&NBPROGRAMS);
	displayRank = realRank + 1;

	long double averageThreadTime;
	long double * threadTimes;
	int offset, rest, thread_id;
	int * itertab;
	//FILE *file;
	long double totalTime, timerStart, timerEnd;

	nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
	nbpixely = ceil((YMAX - YMIN) / RESOLUTION);

	offset = nbpixelx / NBPROGRAMS;
	rest = nbpixelx - NBPROGRAMS * offset;

	printf("\nMe ... : (%d/%d)\nPixels .... : %d\nOffset .... : %d\nRest ...... : %d\n", displayRank, NBPROGRAMS, nbpixelx, offset, rest );

	/*
	 * Allocation du tableau de pixel
	 */
	//TODO: Pour l'instant chaque programme va allouer autant de mémoire qu'il faut pour tous les pixels. Il faudra
	//que chaque programmes s'allouent uniquement la mémoire dont il a besoin
	if ((itertab = malloc(sizeof(int) * offset)) == NULL)
	{
		printf("ERREUR d'allocation de itertab[], errno : %d (%s) .\n", errno, strerror(errno));
		exitWithCode(EXIT_FAILURE);
	}
	//if ((threadTimes = malloc( sizeof(long double) * NBTHREAD ) ) == NULL) {
	//	printf("ERREUR d'allocation de threadtimes[], errno : %d (%s) .\n", errno, strerror(errno));
	//	exitWithCode(EXIT_FAILURE);
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
	long double localTime = getMicrotime();
	struct threadArguments threadArgs;
	threadArgs.start = realRank * offset;
	threadArgs.end = threadArgs.start + offset;
	threadArgs.itertab = itertab;
	threadArgs.threadNum = realRank;
	threadArgs.threadTimes = &localTime;
	printf("(%d) Compute from %d to %d\n",displayRank,threadArgs.start,threadArgs.end);
	threadFunction(&threadArgs);
	printf("(%d) Finish\n",displayRank);

	char* fileName = alloca(sizeof("output") + (unsigned)log10(NBPROGRAMS) + 1);
	char* rankInStr = alloca((unsigned)log10(realRank) + 1) + 1;

	strcpy(fileName,"output");
	sprintf(rankInStr,"%d\0",realRank);

	strcat(fileName,rankInStr);
	printf("(%d) Write to '%s'\n",displayRank,fileName);

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
	//if ( WRITE )
	//{
	///**
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
