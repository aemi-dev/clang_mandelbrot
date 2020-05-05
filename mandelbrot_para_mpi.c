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

#define OUTFILE "mandelbrot_para_mpi.out"

long double getMicrotime()
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

//TODO: il faudra changer le nom de ça
long double threadFunction(const int nodeRealNumber, const int xOffset, int* itertab)
{
	const long double timerStart = getMicrotime();

	for (int xpixel = 0; xpixel < xOffset; xpixel++ )
	{
		for (int ypixel = 0; ypixel < nbpixely; ypixel++ )
		{
			double xinit = XMIN + (nodeRealNumber*xOffset + xpixel) * RESOLUTION;
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

	const long double timerEnd = getMicrotime();

	return (timerEnd - timerStart) / 1e6;
}

void exitWithCode (const int exitCode)
{
	MPI_Finalize();
	exit(exitCode);
}

int main(int argc, char **argv)
{
	int NBPROGRAMS, realRank;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&realRank);
	MPI_Comm_size(MPI_COMM_WORLD,&NBPROGRAMS);
	const int displayRank = realRank + 1;

	int xOffset, rest;
	int * itertab;
	FILE *file;
	//long double totalTime, timerStart, timerEnd;

	nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
	nbpixely = ceil((YMAX - YMIN) / RESOLUTION);

	xOffset = nbpixelx / NBPROGRAMS;
	rest = nbpixelx - NBPROGRAMS * xOffset;

	printf("\nMe ... : (%d/%d)\nPixels .... : %d\nxOffset .... : %d\nRest ...... : %d\n", displayRank, NBPROGRAMS, nbpixelx, xOffset, rest );

	/*
	 * Allocation du tableau de pixel
	 * Chaque programmes n'allouent que ce dont il a besoin
	 */
	//TODO: Pour l'instant chaque programme va allouer autant de mémoire qu'il faut pour tous les pixels. Il faudra
	//que chaque programmes s'allouent uniquement la mémoire dont il a besoin
	if ((itertab = malloc(sizeof(int) * xOffset * nbpixely)) == NULL)
	{
		perror("ERROR: ");
		exitWithCode(EXIT_FAILURE);
	}
	
	//La séparation des pixels à calculer est faite de façon débile pour l'instant, en divisant l'espace total des x
	//par le nombre de programmes
	const int start = realRank * xOffset;
	const int end = start + xOffset;
	const long double startTime = getMicrotime();
	printf("(%d) Compute from %d to %d\n",displayRank,start,end);
	const long double computeTime = threadFunction(realRank, xOffset,itertab);
	printf("(%d) Computation finished....took %.2Lfs\n",displayRank,computeTime);

	/**
	 * Les résultats sont tous envoyés au noeud numéro 0 qui va les écrires dans un fichier
	 */

	char* fileName = OUTFILE;

	if (realRank == 0)
	{
		//Le programme 0 va recevoir toutes les données des autres programme et les mettre dans le
		//fichier d'output final
		if ((file = fopen(fileName, "w")) == NULL)
		{
			perror("ERROR: ");
			exitWithCode(EXIT_FAILURE);
		}

		for (int i = 0; i < NBPROGRAMS; ++i)
		{
			//On récupère les données calculées de chaque programme
			if (i > 0)
			{
				printf("(1) Will receive data from (%d)\n",(i+1));
				MPI_Recv(itertab,xOffset*nbpixely,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			}
			
			for (int xpixel = 0; xpixel < xOffset; xpixel++)
			{
				for (int ypixel = 0; ypixel < nbpixely; ypixel++)
				{
					double x = XMIN + (i*xOffset + xpixel) * RESOLUTION;
					double y = YMIN + ypixel * RESOLUTION;
					fprintf(file, "%f %f %d\n", x, y, itertab[xpixel * nbpixely + ypixel]);
				}
				fprintf(file, "\n");
			}
		}

		fclose(file);
	}
	else
	{
		printf("(%d) Will send calculated data to (1)\n",displayRank);
		//Les autres programmes envoie leurs données au programme 0
		MPI_Send(itertab,xOffset * nbpixely,MPI_INT,0,0,MPI_COMM_WORLD);
	}

	const long endTime = getMicrotime();
	if (realRank == 0)
		printf("total time : %.2Lfs\n",(endTime - startTime) / 1e6);

	/* Clean */
	free(itertab);

	printf("(%d) Will finish\n",displayRank);

	/*sortie du programme*/
	exitWithCode(EXIT_SUCCESS);
}
