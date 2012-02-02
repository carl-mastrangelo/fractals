CC=gcc
CFLAGS=-c -g -Wall -Werror -ansi -pedantic
CFLAGS=-c -O2 -ffast-math -mtune=native -msse2 -mfpmath=sse -Wall -Werror -ansi -pedantic -fno-math-errno -funsafe-math-optimizations -ffinite-math-only

LFLAGS=-lpthread -lpng
#LFLAGS=-lpthread

all: fractal.o complex.o heap.o pngout.o
	$(CC) $^ $(LFLAGS) -o fractal

fractal.o: fractal.c complex.h heap.h pngout.h
	$(CC) $(CFLAGS) $< -o $@

complex.o: complex.c complex.h
	$(CC) $(CFLAGS) $< -o $@

list.o: list.c list.h
	$(CC) $(CFLAGS) $< -o $@

heap.o: heap.c heap.h
	$(CC) $(CFLAGS) $< -o $@
	
pngout.o: pngout.c pngout.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
