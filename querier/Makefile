CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: query query2 query3 query4

query: query.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

query2: query2.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

query3: query3.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

query4: query4.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -rf *.o
	rm -rf query query2 query3 query4
