/* query.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * Step 2 of Module 6
 * 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

#include "pageio.h"
#include "webpage.h"
#include "queue.h"
#include "hash.h"
#include "indexio.h"

#define BUFSZ 512
#define MIN_WORD_LENGTH 3
#define MAX_QUERY_WORDS 100

// STRUCTURE DECLARATIONS
// todo: this is used in multiple files, we should refactor into its own module

// queue of document structure
typedef struct queue_of_documents {
    queue_t *qp;
    char *word;
} queue_of_documents_t;

// document structure
typedef struct document {
    int id;
    int count;
} document_t;


// SEARCH FUNCTIONS FOR THE INDEX

// find the correct queue corresponding the word key
// sp is a char* word
// *ep is a queue_of_documents_t
bool document_queue_search(void *ep, const void *sp) {
    queue_of_documents_t *q_docs = (queue_of_documents_t *) ep;
    if (strcmp(sp, q_docs->word) == 0) {
        return true;
    }
    return false;
}

// finds the correct document structure matching the id
// sp is an id
// ep is a document_t type
bool document_word_search(void *ep, const void *sp) {
    int *d = (int *) sp;
    document_t *dp = (document_t *) ep;
    if (dp->id == *d) {
        return true;
    }
    return false;
}


// PARSER FUNCTIONS

/*
 * returns false if string contains non alphabetical characters
 */
bool check_string(char *s) {
    for (int i = 0; s[i]; i++) {
        // exempt whitespace
        if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
            continue;
        }
        if (!isalpha(s[i])) {
            return false;
        }
    }
    return true;
}

/*
 * Checks if the string has alphanumeric characters - if it doesn't then no point parsing the query
 */
bool has_alpha(char *s) {
    for (int i = 0; s[i]; i++) {
        if (isalpha(s[i])) {
            return true;
        }
    }
    return false;
}

int query_search(char *word, int id, hashtable_t *index) {
    // search for the presence of the word in the hashtable
    queue_of_documents_t *temp = hsearch(index, &document_queue_search, word, strlen(word));
    if (temp == NULL) {
        // then the word was not found in any of the documents in the index
        return 0;
    }
    else {
        document_t *temp_doc;
        if ((temp_doc = qsearch(temp, &document_word_search, &id)) == NULL) {
            return 0;
        }
        return temp_doc->count;
    }
}

/*
 *  Find the minimum rank given an array of non-negative values for the rank
 */
int minimum_rank(int arr[], int len) {
    int smallest = INT_MAX;
    for (int i = 0; i < len; i++) {
        if (arr[i] == -1) {
            printf("-1 should not occur\n");
            return -1;
        }
        if (arr[i] < smallest) {
            smallest = arr[i];
        }
    }
    return smallest;
}

/*
 * takes in a valid query as user input
 * a valid query has alphabetical characters and whitespace in the form of " ", "\t", or "\n"
 * returns array of strings
 * each string is either a word or a boolean operator AND or OR
 * MEMORY: caller needs to free the array of strings, as well as the strings themselves
 * query_count is the number of query tokens
 */
char **parse_query(char *s, int *query_count) {
    // strip the last character which is a newline in the query
    s[strcspn(s, "\n")] = '\0';

    // first, convert all characters to lowercase
    for (int i = 0; s[i]; i++) {
        s[i] = tolower(s[i]);
    }

    char **query_words = calloc(MAX_QUERY_WORDS, sizeof(char *));

    // split the query into tokens delimited by whitespace
    int i = 0;
    char *splitter = " \t";
    char *token = strtok(s, splitter);
    while (token) {
        // check if the token is of length less than 2 and it's not the special operator OR
        if ((strlen(token) < MIN_WORD_LENGTH) && (strcmp(token, "or") != 0)) {
            token = strtok(NULL, splitter);
            continue;
        }

        // for now, ignore all ands and ors
        if (strcmp(token, "or") == 0 || strcmp(token, "and") == 0) {
            token = strtok(NULL, splitter);
            continue;
        }

        // malloc a portion on the heap for the string
        char *word = malloc(strlen(token) + 1);
        strcpy(word, token);
        query_words[i++] = word;
        token = strtok(NULL, splitter);
    }
    *query_count = i;
    return query_words;
}


// MAIN FUNCTION

int main() {

    // for task 2, only page 1 has been scanned
    char *doc1 = "./indexnm";
    int id = 1;

    // load up the index 
    hashtable_t *index = indexload(doc1);

    // static buffer to hold the query - assume it won't overflow BUFSZ
    char textbuf[BUFSZ];
    printf("> ");

    // each iteration of the while loop analyzes one query
    while (fgets(textbuf, BUFSZ, stdin) ) {
        // analyze the string read in
        // check if buffer read in a new line
        if (has_alpha(textbuf)) {
            if (!check_string(textbuf)) {
                printf("[invalid query]\n");
            }
            else {

                int query_count;
                char **query = parse_query(textbuf, &query_count);

                // allocate an arbitrary initial size
                char *query_msg = calloc(5, 5);
                char buffer[64];

                // -1 is an invalid value in this context - count can never be -1
                // used to terminate the array for the max function
                int *rank_arr = malloc(sizeof(int) * query_count);

                // for good measure, initialize all to -1
                for (int i = 0; i < query_count; i++) {
                    rank_arr[i] = -1;
                }

                // counter to check query_count is the same as i
                int q_count = 0;
                for (int i = 0; query[i] != 0; i++) {
                    char *word = query[i];
                    // search for that word in the index
                    int count = query_search(word, id, index);
                    rank_arr[i] = count;
                    sprintf(buffer, "%s:%d ", word, count);
                    query_msg = realloc(query_msg, strlen(query_msg) + strlen(buffer) + 1);
                    if (query_msg == NULL) {
                        printf("realloc failed\n");
                        exit(EXIT_FAILURE);
                    }
                    strcat(query_msg, buffer);
                    q_count = i;
                }
                
                // if it's just OR and AND
                if (query_count > 0) {
                    assert(query_count - 1 == q_count);

                    // find the minimum of the ranks
                    int min_rank = minimum_rank(rank_arr, query_count);

                    sprintf(buffer, "--rank: %d", min_rank);
                    query_msg = realloc(query_msg, strlen(query_msg) + strlen(buffer) + 1);
                    if (query_msg == NULL) {
                        printf("realloc failed\n");
                        exit(EXIT_FAILURE);
                    }
                    strcat(query_msg, buffer);

                    assert(min_rank != -1);

                    printf("%s\n", query_msg);
                }

                // free the query and its associated strings
                for (int i = 0; query[i] != 0; i++) {
                    free(query[i]);
                }
                free(query);
                free(rank_arr);
                free(query_msg);
            }
        }
        printf("> ");
    }
    indexclose(index);
    exit(EXIT_SUCCESS);
}


