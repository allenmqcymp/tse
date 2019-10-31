/* 
 * pageio.c --- Implements the functions specified in pageio.h
 * 
 * Author: Allen Ma, Stjepan Vrbic
 * Created: Fri Oct 25 08:30:15 2018 (-0400)
 * Version: 1.0
 * 
 * Description: pagesave saves an existing webpage to a file with a
 * numbered name (e.g. 1,2,3 etc); pageload creates a new page by
 * loading a numbered file. For pagesave, the directory must exist and
 * be writable; for loadpage it must be readable.
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

// uncomment to print debug messages
// #define DEBUG 0

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
	char *new_dirname = malloc(sizeof(char) * strlen(dirname) + 1);
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
    fprintf(f, "%s", html);
    fclose(f);
	free(new_dirname);
	free(fname);
    return 0;
}


/* 
 * pageload -- loads the numbered filename <id> in direcory <dirnm>
 * into a new webpage
 * 
 * returns: non-NULL for success; NULL otherwise
 * Caller needs to free the html inside the returned allocated by this function
 */
webpage_t *pageload(int id, char *dirnm) {

    // this means that the maximum id can have 32 digits
    int max_id_len = 32;

    // strip off the trailing slash of dirname, if it exists
    char *lastchar =  &dirnm[strlen(dirnm) - 1];
	char *new_dirname = malloc(sizeof(char) * strlen(dirnm) + 1);
    if (strcmp("/", lastchar) == 0) {
        strcpy(new_dirname, dirnm);
		new_dirname[strlen(new_dirname)-1] = 0;
    }
	else {
		strcpy(new_dirname, dirnm);
	}

    char *fname = malloc(sizeof(char) * strlen(new_dirname) + sizeof(char) * max_id_len);
    sprintf(fname, "%s/%d", new_dirname, id);

    // check if it's possible to write to the directory
	if (access(fname, R_OK) != 0) {
        free(fname);
        free(new_dirname);
        return NULL;
	}

    // open the file, check that it is readable
    FILE *f = fopen(fname, "r");    

    #ifdef DEBUG
        printf("loading file: %s\n", fname);
    #endif

    if (f == NULL) {
        printf("failed to open file %s\n", fname);
        free(fname);
        free(new_dirname);
        return NULL;
    }

    free(fname);
    free(new_dirname);


	// read in the url
	char url_s[512];
	if ((fgets(url_s, sizeof(url_s), f)) == NULL) {
	    printf("failed to read url\n");
	    printf("error no %s\n", strerror(errno));
	    return NULL;
	}
    url_s[strcspn(url_s, "\n")] = '\0';

	// read in the depth, and convert it to an integer
	char depth_buf[128];
	if (fgets(depth_buf, sizeof(depth_buf), f) == NULL) {
	    printf("failed to read depth\n");
	    printf("error no %s\n", strerror(errno));
	    return NULL;
	}

    unsigned long depth = strtoul(depth_buf, NULL, 10);

    char temp_buf[128];
    // read another line the len of the html - we don't really need it
    fgets(temp_buf, sizeof(temp_buf), f);


    // read in the html

	// // position stream pointer to the start of the file
	// fseek(f, 0, SEEK_SET);
    // first read in the 3 lines and discard them
    int skip_count_html = 0;
    int sz = 32;
    char *html_buf = malloc(sz);
    int c;
    int i = 0;
    int buf_idx = 0;
    if (html_buf == NULL) {
        printf("failed to allocate memory to store html\n");
        return NULL;
    }
    while ((c = fgetc(f)) != EOF) {
        if (feof(f)) {
            break;
        }
        char cc = (char) c;
        if (i >= skip_count_html) {
            // check if the buffer is full, if so, expand it
            if (buf_idx >= sz - 1) {
                sz = sz + 32;
                void *try_ptr = realloc(html_buf, sz);
				if (try_ptr == NULL) {
					printf("realloc failed\n");
					return NULL;
				}
				html_buf = try_ptr;
            }
            // read the line into the buffer
            html_buf[buf_idx++] = cc;
        }
        if (strcmp(&cc, "\n") == 0) {
            i++;
        }
    }
    html_buf[buf_idx] = '\0';

    #ifdef DEBUG 
        printf("the html associated with the url- %s -is\n%s", url_s, html_buf);
    #endif

    // construct a new webpage
    webpage_t *pg = webpage_new(url_s, depth, html_buf);
    fclose(f);
    return pg;
}
