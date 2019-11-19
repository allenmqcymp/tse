/* parallel crawler.c ---
 * 
 * 
 * Author: Allen Ma, Stjepan Vrbic, Joe Signorelli
 * Created: Fri 15 November 2019
 * Version: 
 * 
 * Description: Same functionality as crawler but uses pthreads to multithread the code,
 * thereby allowing for a faster crawler
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


#include <sys/types.h>
#include <sys/syscall.h>

#include "webpage.h"
#include "lqueue.h"
#include "lhash.h"
#include "pageio.h"

typedef struct args {
    int id;
} args_t;


lqueue_t *url_queue;
lhashtable_t *url_hashtable;

int id;

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
int num_finished_threads = 0;

int MAXDEPTH;
int NUM_THREADS;
char *pagedir;

bool url_search(void *page_url, const void *search_url) {
	char *p_url = (char *) page_url;
	char *s_url = (char *) search_url;
	if (strcmp(p_url, s_url) == 0) {
		return true;
	}
	return false;
}

void *perform_crawl(void *st) {

    printf("entering perform_crawl function, thread id %d\n", id);

    args_t *st_i = (args_t *) st;
    int tid = st_i->id;

    bool did_think_finish = false;

    webpage_t *pg;
    while (num_finished_threads < NUM_THREADS) {

        // printf("TOP: %d, finished %d\n", tid, num_finished_threads);
        fflush(stdout);

        // check if global_mutex unlocked
        pg = lqget(url_queue);

        if (pg == NULL) {
            // lqlen acquires mutex, may cause issues
            // printf("thread id %ld, pg is NULL, qlen is %d\n\n", pthread_self(), lqlen(url_queue));
            fflush(stdout);
            if (!did_think_finish) {
                printf("ID: %d IS NOW IDLE\n", tid);
                num_finished_threads++;
            }
            else {
                // printf("ID: %d already idle\n", tid);
            }
            did_think_finish = true;
            continue;
        }
        else {
            if (did_think_finish) {
                printf("ID: %d REACTIVATED, pg_url from queue is %s\n", tid, webpage_getURL(pg));
                fflush(stdout);
                num_finished_threads--;
            }
            else {
                printf("ID: %d already active, pg_url from queue is %s\n", tid, webpage_getURL(pg));
            }
            did_think_finish = false;
        }

        int depth = webpage_getDepth(pg);

        if (depth >= MAXDEPTH) {
            printf("ID: %d, maxdepth %d reached, so continuing\n", tid, depth);
            continue;
        }
        else {
            printf("ID: %d, preparing to expand %s\n", tid, webpage_getURL(pg));
        }
        
        char *q_url;
        int pos = 0;
        while ((pos = webpage_getNextURL(pg, pos, &q_url)) > 0) {
            fflush(stdout);
            if (!IsInternalURL(q_url)) {
                free(q_url);
                continue;
            }

            printf("ID: %d, during expansion of %s, found %s\n", tid, webpage_getURL(pg), q_url);
            
            bool added = lhsearch_add(url_hashtable, url_search, q_url, strlen(q_url));
            if (added) {
                webpage_t *new_pg = webpage_new(q_url, depth + 1, NULL);
                bool res = webpage_fetch(new_pg);
                if (!res) {
                    printf("failed to fetch html for %s\n", q_url);
                }
                
                printf("ID %d, saving and adding to queue %s, id: %d\n", tid, q_url, id);
                int val = lqput(url_queue, new_pg);
                if (val != 0) {
                    printf("lqput failed\n");
                }
                id++;
                printf("SAVING ID: %d\n", id);
                pagesave(new_pg, id, pagedir);

            }
            else {
                printf("ID %d, already seen %s\n", tid, q_url);
                free(q_url);
            }
        }
        printf("BOTTOM: %d, num_finished_threads %d\n", tid, num_finished_threads);
        fflush(stdout);
    }
    
    return NULL;
}

int main(int argc, char * argv[]) {

	if (argc < 5) {
        printf("Usage: ./crawler6 <seedurl> <pagedir> <maxdepth> <num_threads>\n");
        exit(EXIT_FAILURE);
    }

    
    char *seed_url = malloc(strlen(argv[1]) + 1);
    strcpy(seed_url, argv[1]);

    char *tempdir = argv[2];
    pagedir = malloc(strlen(tempdir) + 1);
    strcpy(pagedir, tempdir);

    MAXDEPTH = atoi(argv[3]);
    if (MAXDEPTH < 0) {
        printf("max depth cannot be less than 0\n");
        exit(EXIT_FAILURE);
    }

    printf("maxdepth %d\n", MAXDEPTH);

    NUM_THREADS = atoi(argv[4]);
    if (NUM_THREADS < 1) {
        printf("number of threads: %d must be 1 or more\n", NUM_THREADS);
        exit(EXIT_FAILURE);
    }

    printf("number of threads: %d specified\n", NUM_THREADS);

	webpage_t *seed_page = webpage_new(seed_url, 0, NULL);
    
    struct stat sb;
    if (stat(pagedir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        printf("%s is not a valid directory, so making it\n", pagedir);
        mkdir(pagedir, 0700);
    }

	if (seed_page == NULL) {
		printf("seed page is null\n");
		exit(EXIT_FAILURE);
	}
	
	if (!webpage_fetch(seed_page)) {
		printf("failed to fetch seed page html\n");
		exit(EXIT_FAILURE);
	}

	url_queue = lqopen();
    url_hashtable = lhopen(1000);

    
    lqput(url_queue, seed_page);
    lhput(url_hashtable, seed_url, seed_url, strlen(seed_url));

    id = 1;

    
    printf("saving url %s, id: %d\n", seed_url, id);
    pagesave(seed_page, id, pagedir);

    printf("just before thread creation\n");

    pthread_t thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        args_t *st_args = malloc(sizeof(args_t));
        st_args->id = i;
        printf("%d\n", st_args->id);
        int val = pthread_create(&thread_ids[i], NULL, perform_crawl, st_args);
        if (val != 0) {
            printf("thread creation failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // need to free the id_structs
    
    for (int i = 0; i < NUM_THREADS; i++) {
        int val = pthread_join(thread_ids[i], NULL);
        if (val != 0) {
            printf("thread joining failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    printf("finished crawl, freeing stuff now\n");

    
    lhapply(url_hashtable, free);
    lhclose(url_hashtable);
    lqapply(url_queue, free);
    lqclose(url_queue);
    free(pagedir);

	exit(EXIT_SUCCESS);
}


