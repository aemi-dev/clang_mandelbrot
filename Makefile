EXEC = para.out seq.out

.PHONY: all run

all: $(EXEC)

run: para.out seq.out
	./seq.out
	./para.out

para.out: mandelbrot_para.c config.h
	$(CC) -lpthread -lm -Wall -pedantic $< -o $@

seq.out: mandelbrot_sequentiel.c config.h
	$(CC) -lpthread -lm -Wall -pedantic $< -o $@