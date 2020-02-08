EXEC = seq.out para.out omp.out wp.out

.PHONY: all run

all: $(EXEC)

run: para.out seq.out omp.out wp.out
	./seq.out
	./para.out
	./omp.out
	./wp.out

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

para.out: mandelbrot_para.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

omp.out: mandelbrot_para_omp.c config.h
	$(CC) -Wall -pedantic $< -o $@ -lm -fopenmp

wp.out: mandelbrot_para_write.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

time: para.out seq.out omp.out
	time ./seq.out
	time ./para.out
	time ./omp.out
	time ./wp.out

