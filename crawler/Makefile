CFLAGS= -Wall -pedantic -std=c11 -I../utils/ -L../lib/ -g
LIBS=-lutils -lcurl
CC=gcc

all: crawler crawler3 crawler5 crawler6


crawler: crawler.c 
	$(CC) $(CFLAGS) crawler.c $(LIBS) -o $@

crawler3: crawler3.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
	

crawler5: crawler5.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

crawler6: crawler6.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@


clean:
	rm -rf *.o
	rm crawler crawler3 crawler5 crawler6
	rm -r *.dSYM
