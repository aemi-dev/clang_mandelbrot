EXEC = seq.out para.out mpi.out omp.out
CPPFLAGS := $(CPPFLAGS) -I/usr/local/opt/llvm/include
LDFLAGS := $(LDFLAGS) -L/usr/local/opt/llvm/lib
MPICC = mpicc

CFLAGS = -O3 -Wall -Wextra -pedantic

ifeq ($(shell uname), "Darwin")
	CC = /usr/local/opt/llvm/bin/clang
endif

.PHONY: clean run time seq para mpi

all: $(EXEC)

seq: seq.out
para: para.out
omp: omp.out
mpi: mpi.out

run: para.out seq.out omp.out mpi.out
	./seq.out
	./para.out
	./omp.out

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) $(CPPFGLAGS) $(CFLAGS) -lm $< -o $@

para.out: mandelbrot_para.c config.h
	$(CC) $(CPPFGLAGS) $(CFLAGS) -pthread -lm $< -o $@

wp.out: mandelbrot_para_write.c config.h
	$(CC) $(CPPFGLAGS) $(CFLAGS) -pthread -lm $< -o $@

omp.out: mandelbrot_para_omp.c config.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -fopenmp -lm $< -o $@

mpi.out: mandelbrot_para_mpi.c config.h
	$(MPICC) $(CPPFGLAGS) $(CFLAGS) $< -o $@

time: $(EXEC)
	for e in $^; echo running $$e; do time ./$$e; done;

clean:
	rm $(EXEC)
