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

	//check if directory exists, if not, create it
	DIR* dir = opendir(new_dirname);
	if (ENOENT == errno){
		//directory does not exist
		closedir(dir);
		mkdir(new_dirname, 0700);
	}

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

    fprintf(f, "%s\n", url);
    fprintf(f, "%d\n", depth);
    fprintf(f, "%d\n", html_len);
    fprintf(f, "%s\n", html);
    fclose(f);
	free(new_dirname);
	free(fname);
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
    qput(url_queue, seed_page);
    hput(url_hashtable, seed_url, seed_url, sizeof(seed_url));

    webpage_t *q_page;
    int id = 1;
    while ((q_page = qget(url_queue)) != NULL) {
        printf("url is %s\n", webpage_getURL(q_page));
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


