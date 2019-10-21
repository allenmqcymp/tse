/* crawler.c ---
 * 
 * 
 * Author: Allen Ma, Stjepan Vrbic, Joe Signorelli
 * Created: Thu Oct 17 13:02:37 2019 (-0400)
 * Version: 
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "webpage.h"
#include "queue.h"

int main(void) {
	char* seed = "https://thayer.github.io/engs50/";
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
	char *url = NULL;
	queue_t *url_queue = qopen();
	while ((pos = webpage_getNextURL(page, pos, &url)) > 0) {
		printf("Found url: %s, internal?: %s\n", url, IsInternalURL(url) ? "YES" : "NO");
		// for each internal url, put it in a queue
		if (IsInternalURL(url)) {
			// create a new webpage
			webpage_t *pg = webpage_new(url, depth + 1, NULL);
			// place it in the queue
			qput(url_queue, pg);
		}
		free(url);
	}
	url = NULL;
	
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
	// close the queue
	qclose(url_queue);

	exit(EXIT_SUCCESS);
}


