/* crawler6.c ---
 * Author: Allen Ma, Stjepan Vrbic
 * Created: Thu Oct 17 13:02:37 2019 (-0400)
 * Version: 
 * 
 * Description: 
 * Crawl all pages reachable from seedurl, following links to a maximum depth of maxdepth
 * all webpages crawled are internal URL
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#include "webpage.h"
#include "queue.h"
#include "hash.h"

bool url_search(void *page_url, const void *search_url) {
	char *p_url = (char *) page_url;
	char *s_url = (char *) search_url;
	if (strcmp(p_url, s_url) == 0) {
		return true;
	}
	return false;
}

int main(int argc, char * argv[]) {
    if (argc < 4) {
        printf("Usage: ./crawler6 <seedurl> <pagedir> <maxdepth>\n");
        exit(EXIT_FAILURE);
    }

    char *seed_url = argv[1];
    char *pagedir = argv[2];
    int maxdepth = atoi(argv[3]);

    // check that pagedir is a valid directory
    struct stat sb;
    if (stat(pagedir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        printf("%s is not a valid directory\n", pagedir);
        exit(EXIT_FAILURE);
    }

    printf("seed_url: %s\n", seed_url);
    printf("pagedir: %s\n", pagedir);
    printf("maxdepth: %d\n", maxdepth);

    assert(maxdepth > 0);

	webpage_t *seed_page = webpage_new(seed_url, 0, NULL);
	if (seed_page == NULL) {
		printf("webpage at seed_url %s is null\n", seed_url);
		exit(EXIT_FAILURE);
	}

	// make a hashtable of visited webpages
	hashtable_t *url_hashtable = hopen(100);
    queue_t *url_queue = qopen();
    
    // add the seed_url to the hashtable, and add the seed_url webpage to the queue
    char *seed_url;
    qput(url_queue, seed_page);
    hput(url_hashtable, seed_url, seed_url, sizeof(seed_url));

    webpage_t *q_page;
    int depth = 0;
    int id = 1;
    while (q_page = qget(url_queue) != NULL) {
        int pos = 0;
        char *q_url = NULL;
        while ((pos = webpage_getNextURL(q_page, pos, &q_url)) > 0) {
            // get the depth of the current webpage
            int curdepth = webpage_getDepth(q_page);

            if (curdepth >= maxdepth) {
                continue;
            }

            if (IsInternalURL(q_url)) {
                // check if the url is in the hashtable
                if (hsearch(url_hashtable, &url_search, q_url, sizeof(q_url)) == NULL) {
                    // add the url to the hashtable
                    hput(url_hashtable, q_url, q_url, sizeof(q_url));
                    // create a new webpage
                    webpage_t *pg = webpage_new(q_url, curdepth + 1, NULL);
                    // place it in the queue
                    qput(url_queue, pg);
                    // save the page under id
                    pagesave(pg, id, pagedir);
                    id++;
                }
            }
        }
    }

    // free the seed page
	webpage_delete(seed_page);
	// close the queue
	qclose(url_queue);
	// close the hashtable
	hclose(url_hashtable);

    exit(EXIT_SUCCESS);
}


