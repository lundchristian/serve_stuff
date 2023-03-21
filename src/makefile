CC=gcc
CFLAGS=-I./headers

all: app

app: app.o
	$(CC) -o app app.o

app.o: app.c
	$(CC) $(CFLAGS) -c app.c

clean:
	rm -f *.o app