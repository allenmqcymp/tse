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
#include <dirent.h>
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

    //pagep = webpage_new()

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

    char *fname = malloc(sizeof(char) * strlen(new_dirname) + sizeof(char) * max_id_len);
    sprintf(fname, "%s/%d", new_dirname, id);

	//check if directory exists, if not, create it
	DIR* dir = opendir(new_dirname);
	if (ENOENT == errno){
		//directory does not exist
		mkdir(new_dirname, 0700);
	}
	closedir(dir);

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


int main(int argc, char * argv[]) {

	if (argc < 4) {
        printf("Usage: ./crawler6 <seedurl> <pagedir> <maxdepth>\n");
        exit(EXIT_FAILURE);
    }

    char *seed_url = malloc(sizeof(char) * strlen(argv[1]));
    strcpy(seed_url, argv[1]); 
    char *pagedir = argv[2];
    int maxdepth = atoi(argv[3]);
	webpage_t *seed_page = webpage_new(seed_url, 0, NULL);
    // check that pagedir is a valid directory
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

	// scanned the fetched html for urls and print whether the url is internal or not
	int pos = 0;
	queue_t *url_queue = qopen();

    int id = 1;

	//save the seed page page
	int32_t res = pagesave(seed_page, id, "../pages/");
	if (res != 0){
		printf("failed to save page");
	}else{
        id++;
    }

	// make a hashtable of visited webpages
	hashtable_t *url_hashtable = hopen(1000);


    //put the seed page into the hash and the queue
    qput(url_queue, seed_page);
    hput(url_hashtable, seed_url, seed_url, sizeof(seed_url));

    webpage_t *q;
    while ((q = (webpage_t *)qget(url_queue)) != NULL){
        int pos = 0;
        char *q_url = NULL;
        pos = webpage_getNextURL(q, pos, &q_url);
        while (pos > 0) {
            fflush(stdout);
            int depth = webpage_getDepth(q);
            if (depth > maxdepth){
                break;
            }
            
            if (IsInternalURL(q_url)){
                // check if the url is in the hashtable
                if (hsearch(url_hashtable, &url_search, q_url, sizeof(q_url)) == NULL) {
                    // add the url to the hashtable
                    hput(url_hashtable, q_url, q_url, sizeof(q_url));
                    // create a new webpage
                    webpage_t *pg = webpage_new(q_url, depth + 1, NULL);
                    webpage_fetch(pg);
                    // place it in the queue
                    if (webpage_getHTMLlen(pg) != 47){
                        qput(url_queue, pg);
                        // save the page under id
                
                        pagesave(pg, id, pagedir);
                    }
                    id++;
                }else{
                    
                    free(q_url);
                }
            }
            pos = webpage_getNextURL(q, pos, &q_url);
        }
        
        free(q_url);
    }
    // free the seed page
	webpage_delete(seed_page);
	// close the queue
	qclose(url_queue);
	// close the hashtable
	hclose(url_hashtable);

	exit(EXIT_SUCCESS);
}
