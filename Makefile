EXEC = para.out seq.out

.PHONY: all run

all: $(EXEC)

run: para.out seq.out
	./seq.out
	./para.out

para.out: mandelbrot_para.c config.h
	$(CC) -Wextra -pedantic $< -lpthread -lm -o $@

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) -Wextra -pedantic $< -lpthread -lm -o $@

time: para.out seq.out
	time ./para.out
	time ./seq.out

omp.out: mandelbrot_para_omp.c config.h
	export OMP_NUM_THREADS=4
	$(CC) -Wextra -pedantic $< -o $@ -lm -fopenmp