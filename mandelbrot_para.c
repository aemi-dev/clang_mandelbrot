#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "config.h"

#define NBTHREADS 3
#define OUTFILE "mandelbrot_para.out"

typedef struct MandelbrotComputInfo
{
	unsigned int startXPixel;
} MandelbrotComputInfo;

static unsigned int xOffset = 0;

void *mandelbrotComput(void *arg)
{
	MandelbrotComputInfo *info = (MandelbrotComputInfo *)arg;

	for (int xpixel = 0; xpixel < xOffset; xpixel++)
	{
		for (int ypixel = 0; ypixel < nbpixely; ypixel++)
		{
			double xinit = XMIN + (info->startXPixel + xpixel) * RESOLUTION;
			double yinit = YMIN + ypixel * RESOLUTION;
			double x = xinit;
			double y = yinit;
			//printf("x: %f   y: %f\n", x, y);
			int iter = 0;
			for (iter = 0; iter < NITERMAX; iter++)
			{
				double prevy = y, prevx = x;
				if ((x * x + y * y) > 4)
					break;
				x = prevx * prevx - prevy * prevy + xinit;
				y = 2 * prevx * prevy + yinit;
			}

			const unsigned int writeIndex = (xpixel + info->startXPixel) * nbpixely + ypixel;
			itertab[writeIndex] = iter;
		}
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	FILE *file;
	pthread_t threadID [NBTHREADS];
	MandelbrotComputInfo infos [NBTHREADS];

	/*calcul du nombre de pixel*/
	nbpixelx = CEIL((XMAX - XMIN) / RESOLUTION);
	nbpixely = CEIL((YMAX - YMIN) / RESOLUTION);

	xOffset = nbpixelx / NBTHREADS;
	printf("nbpixelx: %d   xPixel: %d   xOffset: %f\n",nbpixelx,xOffset,xOffset*RESOLUTION);

	/*allocation du tableau de pixel*/
	if ((itertab = malloc(sizeof(int) * nbpixelx * nbpixely)) == NULL)
	{
		printf("ERREUR d'allocation de itertab, errno : %d (%s) .\n", errno, strerror(errno));
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < NBTHREADS; ++i)
	{
		MandelbrotComputInfo* info = &infos[i];
		info->startXPixel = i*xOffset;

		const int rc = pthread_create(&threadID[i],NULL,mandelbrotComput,(void*)info);
		if (rc != 0)
		{
			fprintf(stderr,"Erreur à la création du thread %zu\n", i);
			//Attente de la fin des threas précédemment lancées
			for (size_t j = 0; j < i; ++j)
				pthread_join(threadID[j], NULL);
			return EXIT_FAILURE;
		}
	}

	for (size_t i = 0; i < NBTHREADS; ++i)
		pthread_join(threadID[i], NULL);

	/*output des resultats compatible gnuplot*/
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

	/*clean*/
	free(itertab);

	/*sortie du programme*/
	return EXIT_SUCCESS;
}
