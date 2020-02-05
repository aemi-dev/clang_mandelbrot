#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "config.h"

#define NBTHREAD 512
#define OUTFILE "mandelbrot_para.out"

struct threadArguments
{
	int threadNum;
	int start;
	int end;
	int * itertab;
};

void * threadFunction(void * threadArgs)
{

	struct threadArguments *args = (struct threadArguments *)threadArgs;
	int xpixel = args->start;
	int * itertab = args->itertab;

	// printf("#%d: %f --> %f\n", args->threadNum, XMIN + args->start * RESOLUTION, XMIN + (args->end - 1) * RESOLUTION);

	for ( ; xpixel < args->end; xpixel++ )
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
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{

	FILE *file;
	int thread_id;
	pthread_t thread[NBTHREAD];
	struct threadArguments threadArgs[NBTHREAD];

	nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
	nbpixely = ceil((YMAX - YMIN) / RESOLUTION);

	int offset = nbpixelx / NBTHREAD;

	/*
	 * Allocation du tableau de pixel
	 */
	if ((itertab = malloc(sizeof(int) * nbpixelx * nbpixely)) == NULL)
	{
		printf("ERREUR d'allocation de itertab, errno : %d (%s) .\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	for ( thread_id = 0; thread_id < NBTHREAD; thread_id++)
	{
		int rc = 0;

		threadArgs[thread_id].start = thread_id * offset;
		threadArgs[thread_id].end = thread_id * offset + offset;
		threadArgs[thread_id].threadNum = thread_id;
		threadArgs[thread_id].itertab = itertab;

		rc = pthread_create(&thread[thread_id], NULL, threadFunction, (void *)&threadArgs[thread_id]);
		if (rc != 0)
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

	if ( writing ) {

		/*
	 * Output des resultats compatible GNUPLOT
	 */
		if ((file = fopen(OUTFILE, "w")) == NULL)
		{
			printf("Erreur à l'ouverture du fichier de sortie : errno %d (%s) .\n", errno, strerror(errno));
			return EXIT_FAILURE;
		}

		for (int xpixel = 0; xpixel < nbpixelx; xpixel++)
		{
			for (int ypixel = 0; ypixel < nbpixely; ypixel++)
			{
				double x = XMIN + xpixel * RESOLUTION;
				double y = YMIN + ypixel * RESOLUTION;
				fprintf(file, "%f %f %d\n", x, y, itertab[xpixel * nbpixely + ypixel]);
			}
			fprintf(file, "\n");
		}
		fclose(file);

	}

	/* Clean */
	free(itertab);

	/*sortie du programme*/
	return EXIT_SUCCESS;
}
