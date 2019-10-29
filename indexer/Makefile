CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: indexer indexer4 indexer5


indexer: indexer.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

indexer4: indexer4.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

indexer5: indexer5.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -rf *.o
	rm -rf indexer indexer4 indexer5
	rm -r *.dSYM
