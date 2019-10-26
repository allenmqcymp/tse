/* crawler.c ---
 * 
 * 
 * Author: Allen Ma, Stjepan Vrbic, Joe Signorelli
 * Created: Thu Oct 17 13:02:37 2019 (-0400)
 * Version: 
 * 
 * Description: hashtable of urls
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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

int main(void) {

	char* seed = "https://thayer.github.io/engs50";
	printf("%d", sizeof(char) * strlen(seed));
	webpage_t *page = webpage_new(seed, 0, NULL);
	int depth = 0;
	if (page == NULL) {
		printf("webpage is null\n");
		exit(EXIT_FAILURE);
	}

	if(!webpage_fetch(page)) {
		printf("failed to fetch html for page\n");
		exit(EXIT_FAILURE);
	}
	
	// scanned the fetched html for urls and print whether the url is internal or not
	int pos = 0;
	char *url;
	queue_t *url_queue = qopen();

	// make a hashtable of visited webpages
	hashtable_t *url_hashtable = hopen(100);
	pos = webpage_getNextURL(page, pos, &url);
	while (pos > 0) {
		// for each internal url, put it in a queue
		if (IsInternalURL(url)) {
			// check if the url is in the hashtable
			if (hsearch(url_hashtable, &url_search, url, sizeof(url)) == NULL) {
				// add the url to the hashtable
				// make a url type
				hput(url_hashtable, url, url, sizeof(url));
				// add the webpage to the queue
				// create a new webpage
				webpage_t *pg = webpage_new(url, depth, NULL);
				// place it in the queue
				qput(url_queue, pg);
			}else{ // if a website is already in the hash table, free the url because it won't be freed later
				free(url);
			}
		}
		pos = webpage_getNextURL(page, pos, &url);
	}
	free(url);
	
	webpage_t *pg = (webpage_t *) qget(url_queue);
	// check that there are no internal urls in the queue
	while (pg != NULL) {
		
		if (!IsInternalURL(webpage_getURL(pg))) {
			printf("%s is not an internal page\n", webpage_getURL(pg));
			exit(EXIT_FAILURE);
		}
		// print out the url of the webpage
		printf("Inside the queue: %s\n", webpage_getURL(pg));
		// free the webpage while we're at it
		webpage_delete(pg);
		pg = (webpage_t *) qget(url_queue);
	}
	// free the seed page
	webpage_delete(page);
	// delete the webpage ptr
	// close the queue
	qapply(url_queue, webpage_delete);
	qclose(url_queue);
	// close the hashtable
	happly(url_hashtable, free);
	hclose(url_hashtable);

	exit(EXIT_SUCCESS);
}


