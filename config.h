#ifndef CONFIG_H
#define CONFIG_H

double XMIN = -2;
double YMIN = -2;
double XMAX = 2;
double YMAX = 2;
double RESOLUTION = 0.7;

int NITERMAX = 100;
int nbpixelx = 0;
int nbpixely = 0;
int* itertab = 0;

#define CEIL(x) (int)((float)(x)+0.5f)

#endif