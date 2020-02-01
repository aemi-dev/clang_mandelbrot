EXEC = para.out seq.out

.PHONY: all run

all: $(EXEC)

run: para.out seq.out
	./seq.out
	./para.out

para.out: mandelbrot_para.c config.h
	clang -O3 -lpthread $< -o $@

seq.out: mandelbrot_sequentiel.c config.h
	clang -O3 $< -o $@