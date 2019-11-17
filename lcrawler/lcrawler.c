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


lqueue_t *url_queue;
lhashtable_t *url_hashtable;

int id;


pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
int num_finished_threads = 0;
bool did_think_finish = false;

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

void *perform_crawl(void *ptr) {

    webpage_t *pg;
    while (num_finished_threads < NUM_THREADS) {

        printf("thread id %ld, num_finished_threads %d\n", pthread_self(), num_finished_threads);

        pthread_mutex_lock(&global_mutex);
        pg = lqget(url_queue);
        if (pg == NULL) {
            num_finished_threads++;
            did_think_finish = true;
        }
        else {
            if (did_think_finish) {
                num_finished_threads--;
                did_think_finish = false;
            }
        }
        pthread_mutex_unlock(&global_mutex);

        if (pg == NULL) {
            printf("thread id %ld, pg is NULL\n", pthread_self());
            continue;
        }

        int depth = webpage_getDepth(pg);

        printf("thread id %ld, depth is %d\n", pthread_self(), depth);

        if (depth >= MAXDEPTH) {
            continue;
        }
        
        char *q_url;
        int pos = 0;
        while ((pos = webpage_getNextURL(pg, pos, &q_url)) > 0) {
            fflush(stdout);
            printf("q_url is %s\n", q_url);
            if (!IsInternalURL(q_url)) {
                free(q_url);
                continue;
            }
            
            bool added = lhsearch_add(url_hashtable, url_search, q_url, strlen(q_url));
            if (added) {
                
                webpage_t *new_pg = webpage_new(q_url, depth + 1, NULL);
                bool res = webpage_fetch(new_pg);
                if (!res) {
                    printf("failed to fetch html for %s\n", q_url);
                }
                int val = lqput(url_queue, new_pg);
                if (val != 0) {
                    printf("lqput failed\n");
                }
                pagesave(new_pg, ++id, pagedir);
                printf("saving %s, id: %d\n", q_url, id);
            }
            else {
                printf("freeing stuff\n");
                free(q_url);
            }
        }
        webpage_delete(pg);
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
        int val = pthread_create(&thread_ids[i], NULL, perform_crawl, NULL);
        if (val != 0) {
            printf("thread creation failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    
    for (int i = 0; i < NUM_THREADS; i++) {
        int val = pthread_join(thread_ids[i], NULL);
        if (val != 0) {
            printf("thread joining failed: %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    printf("freeing stuff now\n");

    
    lhapply(url_hashtable, free);
    lhclose(url_hashtable);
    lqapply(url_queue, free);
    lqclose(url_queue);
    free(pagedir);

	exit(EXIT_SUCCESS);
}