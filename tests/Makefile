CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: test_pageio test_indexio

test_pageio: test_pageio.c 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

test_indexio: test_indexio.c 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@


clean:
	rm -rf *.o
	rm test_pageio test_indexio
