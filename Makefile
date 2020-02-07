EXEC = para.out seq.out omp.out

.PHONY: all run

all: $(EXEC)

run: para.out seq.out omp.out
	./seq.out
	./para.out
	./omp.out

para.out: mandelbrot_para.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

time: para.out seq.out omp.out
	time ./para.out
	time ./seq.out
	time ./omp.out

omp.out: mandelbrot_para_omp.c config.h
	$(CC) -Wall -pedantic $< -o $@ -lm -fopenmp