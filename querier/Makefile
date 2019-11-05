CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: query

query: query.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -rf *.o
	rm -rf query
