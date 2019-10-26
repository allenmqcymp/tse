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
    // save the webpage to another file
    pagesave(test_pg, 2, "../pages/");
    print("html in test pg is %s\n", webpage_getHTML(test_pg));
    // verify that the two files are the same
    system("diff ../pages/1 ../pages/2");
	webpage_delete(test_pg);
    exit(EXIT_SUCCESS);
}
