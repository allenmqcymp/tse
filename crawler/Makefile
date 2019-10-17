CFLAGS= -Wall -pedantic -std=c11 -I. -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: crawler


crawler: 
	$(CC) $(CFLAGS) crawler.c $(LIBS) -o $@
	


clean:
	rm -rf *.o

