CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g -pthread
LIBS=-lutils -lcurl
CC=gcc
TARGETS =lcrawler protocrawler priurl

all: $(TARGETS)


lcrawler: lcrawler.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

protocrawler: protocrawler.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

priurl: print_urls.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@


.PHONY: clean
clean:
	rm -rf *.o
	rm $(TARGETS)
