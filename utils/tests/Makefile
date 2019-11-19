CFLAGS= -Wall -pedantic -std=c11 -I../ -L../../lib/ -pthread -g
CC=gcc
LIBS=-lm -lutils

TEST_LQUEUE_CFILES:=$(wildcard test_lqueue*.c)
LQUEUE_TARGETS:=$(patsubst %.c,%,$(TEST_LQUEUE_CFILES))
LQUEUE_OBJS:=$(patsubst %.c,%.o,$(TEST_LQUEUE_CFILES))

TEST_LHASH_CFILES:=$(wildcard test_lhash*.c)
LHASH_TARGETS:=$(patsubst %.c,%,$(TEST_LHASH_CFILES))
LHASH_OBJS:=$(patsubst %.c,%.o,$(TEST_LHASH_CFILES))

TEST_HASH_CFILES:=$(wildcard test_hash*.c)
HASH_TARGETS:=$(patsubst %.c,%,$(TEST_HASH_CFILES))
HASH_OBJS:=$(patsubst %.c,%.o,$(TEST_HASH_CFILES))

TEST_QUEUE_CFILES:=$(wildcard test_queue*.c)
QUEUE_TARGETS:=$(patsubst %.c,%,$(TEST_QUEUE_CFILES))
QUEUE_OBJS:=$(patsubst %.c,%.o,$(TEST_QUEUE_CFILES))

all: $(LQUEUE_TARGETS) $(LHASH_TARGETS) $(QUEUE_TARGETS) $(HASH_TARGETS)

$(LQUEUE_TARGETS) : %: %.o 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

$(LQUEUE_OBJS) : %.o: %.c 
	$(CC) $(CFLAGS) -c $<

$(LHASH_TARGETS) : %: %.o 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

$(LHASH_OBJS) : %.o: %.c 
	$(CC) $(CFLAGS) -c $<

$(HASH_TARGETS) : %: %.o 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

$(HASH_OBJS) : %.o: %.c 
	$(CC) $(CFLAGS) -c $<

$(QUEUE_TARGETS) : %: %.o 
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

$(QUEUE_OBJS) : %.o: %.c 
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
	rm -rf $(LQUEUE_TARGETS) $(LHASH_TARGETS) $(HASH_TARGETS) $(QUEUE_TARGETS)