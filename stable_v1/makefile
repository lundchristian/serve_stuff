CC=gcc
CFLAGS=-I./headers

all: test

test: test.o
	$(CC) -o test test.o

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -f *.o test