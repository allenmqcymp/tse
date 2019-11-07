/* query.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * Step 3 of Module 6 - basically identical to step 3, but with better code
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

typedef struct document_rank {
    int id;
    int rank;
    int freq;
} document_rank_t;

/*
 * GLOBAL VARIABLES 
 */

// global document hashtable - changed for each query
static hashtable_t *document_rank_table;
static queue_t *q_ranks;

// number of query terms (just AND) - used to filter out terms in the hashtable
static int query_count;

// load up the index 
hashtable_t *index;


/*
 * FUNCTION PROTOTYPES
 */

void intersection_queues(queue_of_documents_t **queues);
char **parse_query(char *s, int *query_count);
char *parse_one_query(char **query_list, char *dirnm);

int compare_ranks(const void *a, const void *b);


// MIN FUNCTION

int min(int a, int b) {
    return a < b ? a : b;
}



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

// char **parse_query_and(char *s) {
//     return NULL;
// }


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
    // we note that or is always surrounded by two spaces
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


/*
 * Parses one query, returns query result
 * Inputs: list of strings representing query lists, ands implicitly added in between spaces; directory name where the pages are stored
 * Outputs: query result to print out
 * Memory: Need to free the output string later
 */
char *parse_one_query(char **query_list, char *dirnm) {
    // counter to check query_count is the same as i
    int q_count = 0;
    bool found_none = false; // flag is set to true when the query word appears in none of the webpages
    // make a list of queue of documents - the size can be up to query_count
    // allocate query_count + 1 because later when we loop through thr word_queues, in intersection_queues, we don't want a null read
    queue_of_documents_t **word_queues = calloc(query_count + 1, sizeof(queue_of_documents_t *));
    for (int i = 0; query_list[i] != 0; i++) {
        char *word = query_list[i];
        q_count = i;
        // search for the queue that contains the word
        queue_of_documents_t *q_word = hsearch(index, &document_queue_search, word, strlen(word));
        if (q_word == NULL) { // the word doesn't exist in the index at all
            found_none = true;
        }
        else {
            word_queues[i] = q_word;
        }
    }

    char *return_msg = malloc(256);

    assert(query_count - 1 == q_count);

    if (found_none) {
        // then don't return anything since no pages match
        sprintf(return_msg, "--------\nno pages found\n");
    }
    else {
        // find the intersection of all the queues
        document_rank_table = hopen(50);
        
        queue_of_documents_t **wq_p = word_queues;
        intersection_queues(wq_p);

        // sort the intersections - add to a list and use qsort to sort by highest ranking
        // choose some arbitrary initial number for the list length
        int curlen = 10;
        document_rank_t *drt;
        document_rank_t **rank_lists = calloc(curlen, sizeof(document_rank_t *));
        int i = 0;
        while ((drt = qget(q_ranks)) != NULL) {
            if (i == curlen - 1) {
                rank_lists = realloc(rank_lists, sizeof(rank_lists) + 10 * sizeof(document_rank_t *));
                if (rank_lists == NULL) {
                    printf("failed to realloc rank_lists\n");
                    exit(EXIT_FAILURE);
                }
                curlen += 10;
            }
            // add it to a list
            rank_lists[i++] = drt;
        }


        qsort((void *) rank_lists, i, sizeof(document_rank_t *), &compare_ranks);

        sprintf(return_msg, "--------\n");
        char temp_buf[128];
        for (int j = 0; j < i; j++) {
            // get the webpage url
            webpage_t *pg = pageload(rank_lists[j]->id, dirnm);
            char *url = webpage_getURL(pg);
            sprintf(temp_buf, "rank: %d, id: %d, url: %s\n", rank_lists[j]->rank, rank_lists[j]->id, url);
            webpage_delete(pg);

            return_msg = realloc(return_msg, strlen(return_msg) + strlen(temp_buf) + 1);
            if (return_msg == NULL) {
                printf("failed to realloc\n");
                exit(EXIT_FAILURE);
            }
            strcat(return_msg, temp_buf);
        }

        // free rank_lists
        free(rank_lists);

        // free the queue
        qapply(q_ranks, free);
        qclose(q_ranks);


        // free the hashtable
        happly(document_rank_table, free);
        hclose(document_rank_table);

    }

    // don't need to free the queues themselves since index is still referencing them
    free(word_queues);

    return return_msg;
    
}

// RANKING FUNCTIONS

bool id_search(void *ep, const void *sp) {
    document_rank_t *rt = (document_rank_t *) ep;
    char strid[32];
    sprintf(strid, "%d", rt->id);
    return strcmp(strid, sp) == 0;
}

void rank_queue(void *ep) {
    document_t *doc = (document_t *) ep;
    // check if ep's id is in the hashtable
    char strid[32];
    sprintf(strid, "%d", doc->id);
    document_rank_t *rt;
    if ( (rt = hsearch(document_rank_table, &id_search, strid, strlen(strid)) ) == NULL) {
        // then we should add it 
        document_rank_t *new_rt = malloc(sizeof(document_rank_t));
        new_rt->freq = 1;
        new_rt->id = doc->id;
        new_rt->rank = doc->count;
        hput(document_rank_table, new_rt, strid, strlen(strid));
    }
    else {
        // increase frequency by 1
        rt->freq += 1;
        // check for the min
        rt->rank = min(rt->rank, doc->count);
    }
}

void extract_query_count(void *ep) {
    document_rank_t *rt = (document_rank_t *) ep;
    if (rt->freq == query_count) {
        qput(q_ranks, rt);
    }
}

/*
 * Finds the intersection of documents specified by queues, which is a list of queue_of_documents
 * Mutates the global variable document_rank_table
 * MEMORY: caller must eventually free the hashtable
 * the list of queues and queues themselves must eventually be freed as well
 */
void intersection_queues(queue_of_documents_t **queues) {
    for (int i = 0; queues[i] != NULL; i++) {
        queue_of_documents_t *cur_q = queues[i];
        qapply(cur_q->qp, &rank_queue);
    }
    // filter out the ones present under all the words
    q_ranks = qopen();
    happly(document_rank_table, &extract_query_count);
}

// comparator function for the sort

int compare_ranks(const void *a, const void *b) {
    document_rank_t *dt_a = *(document_rank_t **) a;
    document_rank_t *dt_b = *(document_rank_t **) b;
    // higher rank should come first
    return -(dt_a->rank - dt_b->rank);
}

// FREE FUNCTIONS




// MAIN FUNCTION

int main() {

    // location of the index file
    char *doc1 = "./indexnm3";
    // where the pages are stored
    char *dirnm = "../pages3/";

    // load up the index 
    index = indexload(doc1);

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

                char **query = parse_query(textbuf, &query_count);

                // if it's just OR and AND in the query string, then query_count will be 0
                // so don't do anything
                if (query_count > 0) {
                    char *res = parse_one_query(query, dirnm);
                    printf("%s", res);
                    free(res);
                }

                // FREE MEMORY

                // free the query and its associated strings
                for (int i = 0; query[i] != NULL; i++) {
                    free(query[i]);
                }
                free(query);
            }
        }
        printf("> ");
    }
    indexclose(index);
    // free the list of queues, and the queues themselves
    exit(EXIT_SUCCESS);
}


