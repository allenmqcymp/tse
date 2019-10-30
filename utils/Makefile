CFLAGS=-Wall -pedantic -std=c11 -I. -g
CC=gcc
LIBS=-lm

OFILES=queue.o hash.o webpage.o pageio.o indexio.o

all: $(OFILES)
	ar cr ../lib/libutils.a $(OFILES)

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $<

webpage.o: webpage.c webpage.h
	$(CC) $(CFLAGS) -c $<

hash.o: hash.c hash.h queue.h
	$(CC) $(CFLAGS) -c $<

pageio.o: pageio.c pageio.h webpage.h
	$(CC) $(CFLAGS) -c $<

indexio.o: indexio.c indexio.h hash.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
