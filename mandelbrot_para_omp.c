#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <omp.h>

#include "config.h"

#define OUTFILE "mandelbrot_para_omp.out"

int main(int argc,char **argv)
{
  int * itertab;
  int xpixel = 0;
  int ypixel = 0;
  FILE * file;

/*calcul du nombre de pixel*/
  nbpixelx = ceil((XMAX - XMIN) / RESOLUTION);
  nbpixely = ceil((YMAX - YMIN) / RESOLUTION);

/*allocation du tableau de pixel*/
  if( (itertab=malloc(sizeof(int)*nbpixelx*nbpixely)) == NULL)
  {
    printf("ERREUR d'allocation de itertab, errno : %d (%s) .\n",errno,strerror(errno));
    return EXIT_FAILURE;
  }

omp_set_num_threads(NBTHREAD);

/*calcul des points*/
  #pragma omp parallel
  {
    #pragma omp for
    for(xpixel=0;xpixel<nbpixelx;xpixel++)
      for(ypixel=0;ypixel<nbpixely;ypixel++) {
        double xinit = XMIN + xpixel * RESOLUTION;
        double yinit = YMIN + ypixel * RESOLUTION;
        double x=xinit;
        double y=yinit;
      //printf("x: %f   y: %f\n", x, y);
  
        int iter=0;
        for(iter=0;iter<NITERMAX;iter++) {
          double prevy=y,prevx=x;
          if( (x*x + y*y) > 4 )
            break;
          x = prevx*prevx - prevy*prevy + xinit;
          y = 2*prevx*prevy + yinit;
        }
        itertab[xpixel*nbpixely+ypixel]=iter;
      }
    }

    if ( WRITE ) {
    /*output des resultats compatible gnuplot*/
      if( (file=fopen(OUTFILE,"w")) == NULL ) {
        printf("Erreur à l'ouverture du fichier de sortie : errno %d (%s) .\n",errno,strerror(errno));
        return EXIT_FAILURE;
      }
      for(xpixel=0;xpixel<nbpixelx;xpixel++) {
        for(ypixel=0;ypixel<nbpixely;ypixel++) {
          double x = XMIN + xpixel * RESOLUTION;
          double y = YMIN + ypixel * RESOLUTION;
          fprintf(file,"%f %f %d\n", x, y,itertab[xpixel*nbpixely+ypixel]);
        }
        fprintf(file,"\n");
      }
      fclose(file);
    }

/*clean*/
    free(itertab);

/*sortie du programme*/
    return EXIT_SUCCESS;
  }
