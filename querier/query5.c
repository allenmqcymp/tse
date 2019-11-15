
/* query5.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * Step 5 of Module 6
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
// mutated every query
static hashtable_t *document_rank_table;

// global queue to hold ids from the document_rank_table
// mutated every query
static queue_t *q_ranks;

// number of query terms (just AND) - used to filter out terms in the hashtable
static int query_count;

// global index - load up indexfile here
static hashtable_t *index;

/*
 * FUNCTION PROTOTYPES
 */

void intersection_queues(queue_of_documents_t **queues);
queue_t *rank_and_query(char **and_query);

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
bool check_invalid_char(char *s) {
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


/*
 * Input: a string that contains query terms with 0 or more of the pattern "<space>or<space>"
 * Returns: - in the case of a valid split: a list of string where each string itself is an AND query
 * NULL if one of the split strings is invalid - ie. it is whitespace
 * Assumptions - the input string has no punctuation and has alphabetical characters
 * MEMORY: if the result is null, then no need to free, otherwise, need to free the list of strings
 */
char **parse_query_or(char *s) {
    // strip the last character which is a newline in the query
    s[strcspn(s, "\n")] = '\0';

    // first, convert all characters to lowercase
    for (int i = 0; s[i]; i++) {
        s[i] = tolower(s[i]);
    }

    char **query_words = calloc(MAX_QUERY_WORDS, sizeof(char *));

    char *temp_buffer = calloc(1, 1);

    int i = 0;
    // we note that or is always surrounded by two spaces
    char *splitter = " \t";
    char *token = strtok(s, splitter);

    int or_count = 0;
    int segment_count = 0;

    while (token) {

        // check if the token is of length less than 2 and it's not the special operator OR
        if ((strlen(token) < MIN_WORD_LENGTH) && (strcmp(token, "or") != 0)) {
            token = strtok(NULL, splitter);
            continue;
        }

        if (strcmp(token, "or") == 0) {
            // add the current temp_buffer to the queue of words
            // only do so if the temp_buffer actually has stuff inside, not just the calloced 0
            if (temp_buffer[0]) {
                segment_count++;
                char *and_string = malloc(strlen(temp_buffer) + 1);
                strcpy(and_string, temp_buffer);
                query_words[i++] = and_string;
                // clear the temp_buffer back to 1
                free(temp_buffer);
                temp_buffer = calloc(1, 1);
            }
            token = strtok(NULL, splitter);
            or_count++;
            continue;
        }

        // plus two because we add a space as well as a EOL character
        temp_buffer = realloc(temp_buffer, strlen(temp_buffer) + strlen(token) + 2);
        if (temp_buffer == NULL) {
            printf("realloc failed when parsing query\n");
            exit(EXIT_FAILURE);
        }
        strcat(temp_buffer, " ");
        strcat(temp_buffer, token);

        token = strtok(NULL, splitter);
    }

    if (temp_buffer[0]) {
        char *final_string = malloc(strlen(temp_buffer) + 1);
        strcpy(final_string, temp_buffer);
        query_words[i++] = final_string;
        segment_count++;
    }

    free(temp_buffer);

    if (or_count + 1 != segment_count) {
        int j = 0;
        while (query_words[j] != 0) {
            free(query_words[j++]);
        }
        free(query_words);
        return NULL;
    }

    return query_words;
}


/*
 * takes in a valid query as user input
 * a valid query has alphabetical characters and whitespace in the form of " ", "\t", or "\n"
 * returns array of strings
 * each string is either a word or a boolean operator AND or OR
 * MEMORY: caller needs to free the array of strings, as well as the strings themselves
 * query_count is the number of query tokens
 */
char **parse_query_and(char *s, int *query_count) {
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
    int and_count = 0;
    int segment_count = 0;
    while (token) {
        if ((strlen(token) < MIN_WORD_LENGTH)) {
            token = strtok(NULL, splitter);
            continue;
        }

        if (strcmp(token, "and") == 0) {
            token = strtok(NULL, splitter);
            and_count++;
            continue;
        }

        // malloc a portion on the heap for the string
        char *word = malloc(strlen(token) + 1);
        strcpy(word, token);
        query_words[i++] = word;
        token = strtok(NULL, splitter);
        segment_count++;
    }

    if (and_count >= segment_count) {
        int j = 0;
        while (query_words[j] != 0) {
            free(query_words[j++]);
        }
        free(query_words);
        *query_count = 0;
        return NULL;
    }

    *query_count = i;
    return query_words;
}


/*
 * Parses one query, returns query result
 * Inputs: list of strings representing query lists, directory name where the pages are stored
 * Outputs: query result to print out
 * Memory: Need to free the output string later
 */
queue_t *rank_and_query(char **query_list) {
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
            break;
        }
        else {
            word_queues[i] = q_word;
        }
    }



    if (!found_none) {
        assert(query_count - 1 == q_count);
        // find the intersection of all the queues
        document_rank_table = hopen(50);
        
        queue_of_documents_t **wq_p = word_queues;
        intersection_queues(wq_p);

        // free the hashtable
        //happly(document_rank_table, free);
        hclose(document_rank_table);
    }

    // don't need to free the queues themselves since index is still referencing them
    free(word_queues);

    if (found_none) {
        return NULL;
    }
    else {
        return q_ranks;
    }    
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
        // make a new copy - so the queue is not related to the hashtable
        document_rank_t *rt_copy = malloc(sizeof(document_rank_t));
        rt_copy->freq = rt->freq;
        rt_copy->id = rt->id;
        rt_copy->rank = rt->rank;
        qput(q_ranks, rt_copy);
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

// UTILITY FUNCTIONS

void add_to_qranks(void *ep) {
    document_rank_t *dt = (document_rank_t *) ep;
    document_rank_t *dt_copy = malloc(sizeof(document_rank_t));
    dt_copy->freq = dt->freq;
    dt_copy->id = dt->id;
    dt_copy->rank = dt->rank;
    qput(q_ranks, dt_copy);
}

/*
 * Assumes q_ranks is non-empty and has stuff to print, prints out as a side effect
 * Takes in a dirnm - the directory where the pages reside
 * Memory: managed by function
 */
void print_results(char *dirnm) {
    // sort the intersections - add to a list and use qsort to sort by highest ranking
    // choose some arbitrary initial number for the list length
    int curlen = 10;
    document_rank_t *drt;
    document_rank_t **rank_lists = calloc(curlen, sizeof(document_rank_t *));
    int i = 0;
    while ((drt = qget(q_ranks)) != NULL) {
        if (i == curlen - 1) {
			curlen += 10;
            rank_lists = realloc(rank_lists, curlen * sizeof(document_rank_t *));
            if (rank_lists == NULL) {
                printf("failed to realloc rank_lists\n");
                exit(EXIT_FAILURE);
            }
            
        }
        // add it to a list
        rank_lists[i++] = drt;
    }

    qsort((void *) rank_lists, i, sizeof(document_rank_t *), &compare_ranks);

    for (int j = 0; j < i; j++) {
        // get the webpage url
        webpage_t *pg = pageload(rank_lists[j]->id, dirnm);
        char *url = webpage_getURL(pg);
        printf("rank: %d, id: %d, url: %s\n", rank_lists[j]->rank, rank_lists[j]->id, url);
        webpage_delete(pg);
        free(rank_lists[j]);
    }

    // free rank_lists
    free(rank_lists);
}


// MAIN FUNCTION

int main(int argc, char *argv[]) {

    char *dirnm = argv[1];
    char *doc1 = argv[2];

    if (argc == 3){
        bool quiet = false;
    }
    else if (argc == 4 && strcmp(argv[3], "-q") == 0){
        bool quiet = true;
    }
    else {
        printf("usage: query <pageDirectory> <indexFile> [-q]");
        exit(EXIT_FAILURE);
    }


    // check if the directory exists, if not exit right away
    // check that pagedir is a valid directory
    struct stat sb;
    if (stat(dirnm, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        printf("%s is not a valid directory\n", dirnm);
        exit(EXIT_FAILURE);
    }

    FILE *file;
    if ((file = fopen(doc1,"r"))!=NULL){
        // file exists
        // load up the index 
        index = indexload(doc1);
        fclose(file);
    }
    else{
        //call indexer
        char command[100];
        strcat(command, "../indexer/indexer7 ");
        strcat(command, dirnm);
        strcat(command, " ");
        strcat(command, "../querier/");
        strcat(command, doc1);
        system(command);
        index = indexload(doc1);
    }


    // static buffer to hold the query - assume it won't overflow BUFSZ
    char textbuf[BUFSZ];
    
    printf("> ");
    // each iteration of the while loop analyzes one query
    while (fgets(textbuf, BUFSZ, stdin) ) {
		bool invalid = false;
        // analyze the string read in
        // check if buffer read in a new line
        if (!check_invalid_char(textbuf)) {
            printf("[invalid query]!\n");
			invalid = true;
        }
        else {

            if (!has_alpha(textbuf)) {
                printf("> ");
                continue;
            }

            char **and_queries = parse_query_or(textbuf);

            if (and_queries == NULL) {
                printf("[invalid query]!\n");
				invalid = true;
            }
            else {

                // make a queue to concatenate all the separate and queries together
                queue_t *q_agg = qopen();

                // corresponding hashtable
                hashtable_t *agg_rank_table = hopen(50);

                // loop through each and query string
                // compute the rank - output as a queue of document ids
                // aggregate queues into q_agg (using qconcat)
                // place all into hashtable with the sum
                // put into list and return the output
                for (int i = 0; and_queries[i] != NULL; i++) {
                    // parse the and query

                    char **and_query_str = parse_query_and(and_queries[i], &query_count);

                    if (and_query_str == NULL) {
                        printf("[invalid query]!\n");
						invalid = true;
                    }
                    else {
                        queue_t *rank_and = rank_and_query(and_query_str);

                        if (rank_and != NULL) {
                            qconcat(q_agg, rank_and);
                        }

                        // free every string
                        for (int j = 0; and_query_str[j] != NULL; j++) {
                            free(and_query_str[j]);
                        }
                        // free the array of strings
                        free(and_query_str);
                        // free the string
                        free(and_queries[i]);
                    }
                }
                        
                document_rank_t *cur_rt;
                while ((cur_rt = qget(q_agg)) != NULL) {

                    // check if cur_rt exists in the hashtable
                    document_rank_t *temp_rt;
                    char strid[32];
                    sprintf(strid, "%d", cur_rt->id);
                    if ((temp_rt = hsearch(agg_rank_table, &id_search, strid, strlen(strid))) == NULL) {
                        document_rank_t *new_rt = malloc(sizeof(document_rank_t));
                        new_rt->freq = cur_rt->freq;
                        new_rt->id = cur_rt->id;
                        new_rt->rank = cur_rt->rank;
                        hput(agg_rank_table, (void *) new_rt, strid, strlen(strid));
                        
                    }
                    else {
                        temp_rt->rank += cur_rt->rank;
                    }
                    free(cur_rt);
                }

                free(and_queries);
                qapply(q_agg, free);
                qclose(q_agg);

                // print stuff out of the hashtable
                // use the queue as temporary storage 

                q_ranks = qopen();
                happly(agg_rank_table, &add_to_qranks);

                printf("--------\n");
            
                document_rank_t *test_item;
                if ((test_item = qget(q_ranks)) == NULL) {
                    printf("nothing found\n");
                }
                else {
                    qput(q_ranks, test_item);
					if (!invalid){
                    	print_results(dirnm);
					}
                }

                qapply(q_ranks, free);
                //qclose(q_ranks);

                // free the hashtable

                //happly(agg_rank_table, free);
                hclose(agg_rank_table);

                
            }
        }
        printf("> ");
    }
    //indexclose(index);
    // free the list of queues, and the queues themselves
    exit(EXIT_SUCCESS);
}



