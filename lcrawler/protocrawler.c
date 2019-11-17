/* parallel crawler.c ---
 * 
 * 
 * Author: Allen Ma, Stjepan Vrbic, Joe Signorelli
 * Created: Fri 15 November 2019
 * Version: 
 * 
 * A really simple multithreaded experiment
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

// debugging purposes
#include <sys/types.h>
#include <sys/syscall.h>

#include "webpage.h"
#include "lqueue.h"
#include "lhash.h"
#include "pageio.h"

// global shared writable resources
// locked queue of urls to visit 
// locked hashtable of urls already visited 
pthread_mutex_t mutexq = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexh = PTHREAD_MUTEX_INITIALIZER;
int count_q = 0;
int count_h = 0;
// shall be incremented everytime a pagesave is invoked
int id;

// readonly
int MAXNUM;

// // search function used to determine if two urls are the same
// bool url_search(void *page_url, const void *search_url) {
// 	char *p_url = (char *) page_url;
// 	char *s_url = (char *) search_url;
// 	if (strcmp(p_url, s_url) == 0) {
// 		return true;
// 	}
// 	return false;
// }

// critical section of code
// function to perform if a url isn't present in hashtable
// in this case, it's saving the page
void add_save_url(void *ep) {
    char *url = (char *) ep;
    printf("saving the page %s at id %d\n", url, id);
    id++;
    // might create a deadlock here if we use lqput
}

bool count_h_thing() {
    pthread_mutex_lock(&mutexh);
    // check if count has already been reached in count_h
    bool added = false;
    if (count_h < count_q) {
        count_h++;
        added = true;
    }
    pthread_mutex_unlock(&mutexh);
    return added;
}


// worker routine for each thread
// essentially:
// if the head of the queue is at maxdepth, then break (since that implies all the rest of the )
// otherwise, try to dequeue a page, get all the urls within the page, and add it into the hashtable
// and save the page
void *perform_crawl(void *ptr) {
    while (1) {
        if (count_q >= MAXNUM) {
            break;
        }
        else {
            bool added = count_h_thing();
            if (added) {
                // this line introduces a bug
                // because count_q must get incremented immediately after, if count_h was previously incremented
                // but as it is now, a thread can alter the contents of count_q
                sleep(1);
                pthread_mutex_lock(&mutexq);
                count_q++;
                pthread_mutex_unlock(&mutexq);
            }
        }
        printf("thread id %ld, count_q is %d, count_h is %d\n", pthread_self(), count_q, count_h);
    }
    return NULL;
}




int main(int argc, char * argv[]) {

	if (argc < 3) {
        printf("Usage: ./protocrawler <MAXNUM> <num_threads>\n");
        exit(EXIT_FAILURE);
    }

    MAXNUM = atoi(argv[1]);
    if (MAXNUM < 1) {
        printf("max number cannot be less than 1\n");
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[2]);
    if (num_threads < 1) {
        printf("number of threads: %d must be 1 or more\n", num_threads);
        exit(EXIT_FAILURE);
    }

    count_h = 0;
    count_q = 1;

    printf("just before thread creation\n");

    // create n threads
    pthread_t thread_ids[num_threads];
    for (int i = 0; i < num_threads; i++) {
        int val = pthread_create(&thread_ids[i], NULL, perform_crawl, NULL);
        if (val != 0) {
            printf("thread creation failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    printf("prints sometime in the future\n");

    // wait for the completion of all the threads
    for (int i = 0; i < num_threads; i++) {
        int val = pthread_join(thread_ids[i], NULL);
        if (val != 0) {
            printf("thread joining failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    printf("freeing stuff now\n");


	exit(EXIT_SUCCESS);
}