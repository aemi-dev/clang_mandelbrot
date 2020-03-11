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
long double threadFunction(const int xOffset, int* itertab)
{
	const long double timerStart = getMicrotime();

	for (int xpixel = 0; xpixel < xOffset; xpixel++ )
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

	const long double timerEnd = getMicrotime();

	return (timerEnd - timerStart) / 1e6;
}

void exitWithCode (const int exitCode)
{
	MPI_Finalize();
	exit(exitCode);
}

void compileAllOutputFileInOneFile (const int totalNumberOfProgram)
{
	FILE* file = fopen(OUTFILE,"w");

	if (file == NULL)
	{
		perror("ERROR: ");
		exitWithCode(EXIT_FAILURE);
	}

	for (int i = 0; i < totalNumberOfProgram; ++i)
	{
		//Même bordel que dans le main pour transformer un numéro en chaîne de caractère
		char* fileName = alloca(sizeof("mpi.output") + ((unsigned)log10(i) + 1) + 1);
		strcpy(fileName,"mpi.output");

		const size_t iStrSize = (unsigned)log10(i) + 1;
		char* realRankInStr = alloca(iStrSize + 1);
		realRankInStr[iStrSize] = '\0';

		sprintf(realRankInStr,"%d",i);

		strcat(fileName,realRankInStr);

		FILE* output_i = fopen(fileName,"r");
		if (output_i == NULL)
		{
			perror("ERROR: ");
			exitWithCode(EXIT_FAILURE);
		}

		//TODO: On pourrait peut être gérer les erreurs ici ?
		char c = '\0';
		while (fread(&c,sizeof(c),1,output_i) > 0)
			fwrite(&c,sizeof(c),1,file);

		fclose(output_i);
	}
	fclose(file);
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
	//	int end = start + xOffset;
//
	//	if ( rest-- > 0 ) {
	//		end = end > nbpixelx ? nbpixelx : end + 1;
	//	}
//
	
	//La séparation des pixels à calculer est faite de façon débile pour l'instant, en divisant l'espace total des x
	//par le nombre de programmes
	const int start = realRank * xOffset;
	const int end = start + xOffset;
	printf("(%d) Compute from %d to %d\n",displayRank,start,end);
	const long double computeTime = threadFunction(xOffset,itertab);
	printf("(%d) Computation finished....took %.2Lfs\n",displayRank,computeTime);

	/**
	 * Chaque programme écrit ses résultats dans un fichier mpi.output<id du programme>
	 * On est en C et transformer l'id du programme en chaîne de caractère va être un peu légèrement
	 * subtilement mais toujours abilement extrêmement casse couille
	 */

	//+1 car (unsigned) joue le rôle de la fonction floor en C et (unsigned)log10([1,2,3,4,5,6,7,8,9]) = 0
	//TODO: pourquoi ça marche pour realRank = 0 ?
	const size_t realRankStringSize = (unsigned)log10(realRank) + 1;

	//On alloue assez d'espace pour stocker la chaîne "mpi.output" et le numéro du programme transformer en chaîne de
	//caractère + 1 pour le caractère nul final
	char* fileName = alloca(sizeof("mpi.output") + realRankStringSize + 1);
	strcpy(fileName,"mpi.output"); //strcpy copie la caractère '\0'

	//Le numéro du processus est convertit en chaîne de caractères
	const size_t realRankInStrSize = (unsigned)log10(realRank) + 1;
	char* realRankInStr = alloca(realRankInStrSize + 1);
	realRankInStr[realRankInStrSize] = '\0';

	//Magiquement ça va écrire realRank dans realRankInStr
	sprintf(realRankInStr,"%d",realRank);

	//On concatène le nom du fichier avec le numéro du programme transformée en chaîne de caractère
	strcat(fileName,realRankInStr);

	//timerEnd = getMicrotime();

	//totalTime = (timerEnd - timerStart) / 1e6;
	//averageThreadTime = 0;
	//for ( size_t i = 0 ; i < NBTHREAD ; i++ ) {
	//	averageThreadTime += threadTimes[i];
	//}

	//printf("Total Time  : %.6Lf seconds\n", totalTime );
	//printf("Thread Time : %.6Lf seconds ( average )\n", averageThreadTime / NBTHREAD );

	/**
	 * Output des resultats compatible GNUPLOT
	 */
	if ((file = fopen(fileName, "w")) == NULL)
	{
		perror("ERROR: ");
		exitWithCode(EXIT_FAILURE);
	}

	for (int xpixel = 0; xpixel < xOffset; xpixel++)
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

	/* Clean */
	free(itertab);

	//Le programme d'id 0 va ensuite compiler tous les fichiers des autres programmes dans un fichier
	//gnu plot final
	/** CETTE FACON DE FAIRE N'EST QUE TEMPORAIRE, LE TEMPS DE COMPRENDRE UN PEU MIEU LE FONCTIONNEMENT 
	 * DE MPI
	 */
	if (realRank == 0)
	{
		printf("(%d) Will compile all the files\n",displayRank);
		compileAllOutputFileInOneFile(NBPROGRAMS);
	}

	printf("(%d) Will finish\n",displayRank);
	/*sortie du programme*/
	exitWithCode(EXIT_SUCCESS);
}
