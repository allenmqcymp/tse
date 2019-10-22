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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

/*
 *
 * This function saves a fetched page in directory dirname with filename designated by id.
 * The content of the file named id should consist of four elements:
 * - the URL that the page was fetched from on one line
 * - the depth assigned to the webpage
 * - the length of the html associated with the page, on one line
 * - the HTML associated with the page
*/ 
int32_t pagesave(webpage_t *pagep, int id, char *dirname) {

	int max_id_len = 32;

    // strip off the trailing slash of dirname, if it exists
    char *lastchar =  &dirname[strlen(dirname) - 1];
	char *new_dirname = malloc(sizeof(char) * strlen(dirname));
    if (strcmp("/", lastchar) == 0) {
        // strip off the last character - set the last character to be NUL 
		strncpy(new_dirname, dirname, sizeof(char) * (strlen(dirname) - 1));
    }
	else {
		strncpy(new_dirname, dirname, sizeof(char) * strlen(dirname));
	}

	printf("got dirname as %s\n", dirname);
	printf("got new dirname as %s\n", new_dirname);

    // get the html from the webpage
    char *html = webpage_getHTML(pagep);
    int html_len = webpage_getHTMLlen(pagep);
    // get the url from the webpage
    char *url = webpage_getURL(pagep);
    // get the depth from the webpage
    int depth = webpage_getDepth(pagep);

    char *fname = malloc(sizeof(char) * strlen(new_dirname) + sizeof(char) * max_id_len);
    sprintf(fname, "%s/%d", new_dirname, id);

	// check if it's possible to write to the directory
	if (access(fname, W_OK) != 0) {
		printf("cannot access %s!!\n", fname);
		chmod(fname, W_OK);
	}

    FILE *f = fopen(fname, "w");
    if (f == NULL) {
        printf("failed to open file %s\n", fname);
		printf("Error %d \n", errno);
        return -1;
    }

    fprintf(f, "%s\n", url);
    fprintf(f, "%d\n", depth);
    fprintf(f, "%d\n", html_len);
    fprintf(f, "%s\n", html);
    fclose(f);
	free(new_dirname);
	free(fname);
    return 0;
}


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

	// make a hashtable of visited webpages
	hashtable_t *url_hashtable = hopen(100);

	// save the page
    int32_t res = pagesave(page, 1, "../tse/pages/");
    if (res == -1) {
        printf("failed to save page\n");
    }

	while ((pos = webpage_getNextURL(page, pos, &url)) > 0) {
		// for each internal url, put it in a queue
		if (IsInternalURL(url)) {
			// check if the url is in the hashtable
			if (hsearch(url_hashtable, &url_search, url, sizeof(url)) == NULL) {
				// add the url to the hashtable
				// make a url type
				hput(url_hashtable, url, url, sizeof(url));
				// add the webpage to the queue
				// create a new webpage
				webpage_t *pg = webpage_new(url, depth + 1, NULL);
				// place it in the queue
				qput(url_queue, pg);
			}
		}
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
	// close the hashtable
	hclose(url_hashtable);

	exit(EXIT_SUCCESS);
}
