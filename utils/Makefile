CFLAGS=-Wall -pedantic -std=c11 -I. -g
CC=gcc
LIBS=-lm

# get all c files whose name starts test. This is used to extract the target name
#https://stackoverflow.com/questions/13995906/get-filename-without-extension-within-makefile
OFILES=queue.o hash.o webpage.o

all: $(OFILES)
	ar cr ../lib/libutils.a $(OFILES)

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $<

webpage.o: webpage.c webpage.h
	$(CC) $(CFLAGS) -c $<

hash.o: hash.c hash.h queue.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
