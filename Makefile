EXEC = seq.out para.out omp.out mpi.out
COMP = /usr/local/opt/llvm/bin/clang
CPPFLAGS = -I/usr/local/opt/llvm/include -fopenmp
LDFLAGS = -L/usr/local/opt/llvm/lib
UNAME := $(shell uname)
MPICC = mpicc

.PHONY: all run time mpi

mpi: mpi.out

all: $(EXEC)

run: para.out seq.out omp.out mpi.out
	./seq.out
	./para.out
	./omp.out

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

para.out: mandelbrot_para.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

omp.out: mandelbrot_para_omp.c config.h

mpi.out: mandelbrot_para_mpi.c config.h
	$(MPICC) $< -o $@

ifeq ($(UNAME),Darwin)
	$(COMP) $(CPPFLAGS) $< -o $@ -lm $(LDFLAGS)
endif
ifeq ($(UNAME),Linux)
	$(CC) -Wall -pedantic $< -o $@ -lm -fopenmp
endif

wp.out: mandelbrot_para_write.c config.h
	$(CC) -Wall -pedantic $< -lpthread -lm -o $@

time: para.out seq.out omp.out
	time ./seq.out
	time ./para.out
	time ./omp.out

