CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -pthread -g
LIBS=-lutils -lcurl
CC=gcc

all: test_thread

test_pageio: test_pageio.c 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

test_indexio: test_indexio.c 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

test_thread: test_thread.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@


clean:
	rm -rf *.o
	rm test_pageio test_indexio test_thread
