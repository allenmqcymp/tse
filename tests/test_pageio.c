/* 
 * pageio.c --- Implements the functions specified in pageio.h
 * 
 * Author: Allen Ma, Stjepan Vrbic
 * Created: Fri Oct 25
 * Version: 1.0
 * 
 * Assumes that there is such a page called 1, it loads the page, then saves it with id 2
 * Then performs a diff command using bash to test if the two files are actually the same
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <webpage.h>
#include <pageio.h>
#include <errno.h>

int main(void) {
    webpage_t *test_pg = pageload(1, "../pages/");
    if (test_pg == NULL) {
        printf("failed to fetch webpage\n");
        exit(EXIT_FAILURE);
    }
    printf("url is %s\n", webpage_getURL(test_pg));
    printf("depth is %d\n", webpage_getDepth(test_pg));
	printf("html is %s\n", webpage_getHTML(test_pg));
	webpage_delete(test_pg);
    exit(EXIT_SUCCESS);
}
