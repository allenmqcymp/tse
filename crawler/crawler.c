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
			webpage_t *pg = webpage_new(url, 0, NULL);
					
		}
		free(url);
	}

		



	webpage_delete(page);

	exit(EXIT_SUCCESS);
}


