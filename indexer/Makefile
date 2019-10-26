CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: indexer


indexer: indexer.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@


clean:
	rm -rf *.o
	rm crawler crawler3 crawler5 crawler6
	rm -r *.dSYM
