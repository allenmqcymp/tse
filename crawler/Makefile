CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: crawler


crawler: crawler.c 
	$(CC) $(CFLAGS) crawler.c $(LIBS) -o $@
	


clean:
	rm -rf *.o
	rm crawler

