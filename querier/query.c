/* query.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * Step 1 of Module 6
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include "pageio.h"
#include "webpage.h"
#include "queue.h"
#include "hash.h"
#include "indexio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSZ 50

char *parse_string(char *);

int main() {

    char *textbuf = malloc(BUFSZ);
    char buffer[BUFSZ];
    printf("> ");
    while( fgets(buffer, BUFSZ, stdin) ) {
        textbuf = realloc( textbuf, strlen(textbuf)+1+strlen(buffer) );
        if( textbuf == NULL ) {
            printf("realloc failed to expand text buffer\n");
            exit(EXIT_FAILURE);
        }
        strcpy( textbuf, buffer );

        // analyze the string read in
        // check if buffer read in a new line
        if (textbuf[0] != '\n') {
            printf("%s\n", parse_string(textbuf));
        }

        printf("> ");
    }
    printf("\n");
    free(textbuf);
    exit(EXIT_SUCCESS);
}

char *parse_string(char *s) {
    // strip all the whitespace, that is, \t, \s from the string
    int count = 0; 
    for (int i = 0; s[i]; i++) {
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') {
            s[count++] = s[i];
        }
    }
    s[count] = '\0';
    // check that the string is purely alphabetical
    for (int i = 0; s[i]; i++) {
        if (!isalpha(s[i])) {
            return "[invalid query]";
        }
        // convert the character to lowercase
        s[i] = tolower(s[i]);
    }
    return s;
}


