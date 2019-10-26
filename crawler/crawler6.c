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
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#include "webpage.h"
#include "queue.h"
#include "hash.h"


/*
 * This function saves a fetched page in directory dirname with filename designated by id.
 * The content of the file named id should consist of four elements:
 * - the URL that the page was fetched from on one line
 * - the depth assigned to the webpage
 * - the length of the html associated with the page, on one line
 * - the HTML associated with the page
*/ 
int32_t pagesave(webpage_t *pagep, int id, char *dirname) {

	printf("in pagesave\n");

	int max_id_len = 32;

    // strip off the trailing slash of dirname, if it exists
    char *lastchar =  &dirname[strlen(dirname) - 1];
	char *new_dirname = malloc(sizeof(char) * strlen(dirname));
    if (strcmp("/", lastchar) == 0) {
        strcpy(new_dirname, dirname);
		new_dirname[strlen(new_dirname)-1] = 0;
    }
	else {
		strcpy(new_dirname, dirname);
	}

    // get the html from the webpage
    char *html = webpage_getHTML(pagep);
    int html_len = webpage_getHTMLlen(pagep);
    // get the url from the webpage
    char *url = webpage_getURL(pagep);
    // get the depth from the webpage
    int depth = webpage_getDepth(pagep);
    
    printf("in pagesave url is %s\n", url);
    printf("in pagesave id is %d\n", id);

    char *fname = malloc(sizeof(char) * strlen(new_dirname) + sizeof(char) * max_id_len);
    sprintf(fname, "%s/%d", new_dirname, id);
    
    printf("inpagesave: new_dirname is %s\n", new_dirname);
    

	// check that new_dirname is a valid directory
    struct stat sb;
    if (stat(new_dirname, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        printf("%s is not a valid directory, so making it\n", new_dirname);
        mkdir(new_dirname, 0700);
    }
    
    printf("in pagesave, fname is %s\n", fname);

	// check if it's possible to write to the directory
	if (access(new_dirname, W_OK) != 0) {
		printf("cannot access %s\n", new_dirname);
		chmod(new_dirname, W_OK);
	}

	//make the file and check if everything went right
    FILE *f = fopen(fname, "w");
    if (f == NULL) {
        printf("failed to open file %s\n", fname);
		printf("Error %d \n", errno);
        return -1;
    }
    
    printf("in pagesave still\n");

    fprintf(f, "%s\n", url);
    fprintf(f, "%d\n", depth);
    fprintf(f, "%d\n", html_len);
    fprintf(f, "%s\n", html);
    fclose(f);
	free(new_dirname);
	free(fname);
	printf("returning out of pagesave\n");
    return 0;
}

/*
 * Search function for hashtable - checks if urls are the same
 */
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

    char *seed_url = malloc(sizeof(char) * strlen(argv[1]));
    strcpy(seed_url, argv[1]); 
    char *pagedir = argv[2];
    int maxdepth = atoi(argv[3]);

    // check that pagedir is a valid directory
    struct stat sb;
    if (stat(pagedir, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        printf("%s is not a valid directory, so making it\n", pagedir);
        mkdir(pagedir, 0700);
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
	
	if (!webpage_fetch(seed_page)) {
		printf("failed to fetch seed page html\n");
		exit(EXIT_FAILURE);
	}

	// make a hashtable of visited webpages
	hashtable_t *url_hashtable = hopen(100);
    queue_t *url_queue = qopen();
    
    // add the seed_url to the hashtable, and add the seed_url webpage to the queue
    qput(url_queue, seed_page);
    hput(url_hashtable, seed_url, seed_url, sizeof(seed_url));
    
    printf("seed page URL is %s\n", webpage_getURL(seed_page));

    webpage_t *q_page = (webpage_t *) qget(url_queue);
    int id = 1;
    
    bool failed = false;
    
    while (q_page != NULL) {
    
        printf("dequeued stuff: url is %s\n", webpage_getURL(q_page));
        char *q_url = NULL;
		if (!webpage_fetch(q_page)) {
			printf("failed to fetch page html\n");
			failed = true;
			break;
		}
		                    
		                    
        // save the page under id
        pagesave(q_page, id, pagedir);
        id++;
                    
		int pos = webpage_getNextURL(q_page, pos, &q_url);
        while (pos > 0) {
        	printf("pos is %d\n", pos);
            // get the depth of the current webpage
            int curdepth = webpage_getDepth(q_page);

            if (curdepth >= maxdepth) {
                continue;
            }
            
            printf("q_url %s\n", q_url);

            if (IsInternalURL(q_url)) {
                // check if the url is in the hashtable
                if (hsearch(url_hashtable, &url_search, q_url, sizeof(q_url)) == NULL) {
                    // add the url to the hashtable
                    hput(url_hashtable, q_url, q_url, sizeof(q_url));
                    // create a new webpage
                    webpage_t *pg = webpage_new(q_url, curdepth + 1, NULL);
                    // place it in the queue
                    printf("adding q_url %s to the queue\n", q_url);
                    qput(url_queue, pg);
                }
            }
            pos = webpage_getNextURL(q_page, pos, &q_url);
        }
        printf("getting another page from the queue\n");
        free(q_url);
        q_page = (webpage_t *) qget(url_queue);
        printf("qpage is %s\n", webpage_getURL(q_page));
    }
    

    
    // free the seed page
	webpage_delete(seed_page);
	// close the queue
	qclose(url_queue);
	// close the hashtable
	hclose(url_hashtable);

    if (failed) {
    	exit(EXIT_FAILURE);
    }
    else {
    	exit(EXIT_SUCCESS);
    }
}


