/* indexer7.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * HTLM indexer - step 7 of module 5
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "pageio.h"
#include "webpage.h"
#include "queue.h"
#include "hash.h"
#include "indexio.h"




// GLOBAL VARIABLES

// global sum variable - gets mutated by each sum_queue, which gets invoked by grand_sum
int sum = 0;

// TYPE DECLARATIONS

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

// MEMORY FREE FUNCTIONS

// frees the word in the queue_of_documents, frees everything inside the queue, 
// frees the queue, and frees the queue_of_documents structure
void free_queues(void *ep) {
    queue_of_documents_t *temp = (queue_of_documents_t *) ep;
    free(temp->word);
    // free every item inside the queue
    qapply(temp->qp, free);
    // free the queue itself
    qclose(temp->qp);
    free(temp);
}

// SEARCH FUNCTIONS

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

// SUM FUNCTIONS

void sum_queue(void *ep) {
    document_t *dp = (document_t *) ep;
    sum += dp->count;
}

void grand_sum(void *ep) {
    queue_of_documents_t *temp = (queue_of_documents_t *)ep;
    qapply(temp->qp, &sum_queue);
}

// helper function
//returns non 0 if the word is alphabetic and >3, returns 0 if the word is rejected
int NormalizeWord(char *word){
    bool alpha = true;
    for (int i = 0; i<strlen(word); i++){
        if (isalpha(word[i]) == 0){
            alpha = false;
            break;
        }else{
            word[i] = tolower(word[i]);
        }
    }
    if (strlen(word) < 3 || !alpha){
        free(word);
        return 0;
    }

    return 1;
}

// MAIN FUNCTION

int main(int argc, char *argv[]){
    char *dir = argv[1];
    char *indexnm = argv[2];

    //make the file and check if everything went right
    FILE *f = fopen("output_file", "w");
    if (f == NULL) {
        printf("failed to open file %s\n", "output_file");
		printf("Error %d \n", errno);
        return -1;
    }

    //make a hashtable to index the occurences of each word
    hashtable_t *index = hopen(1000);
    int idx = 1;
    webpage_t *page = pageload(idx, dir);
    while (page != NULL) {
        printf("index is %d\n", idx);
        int pos = 0;
        char *word = NULL;
        pos = webpage_getNextWord(page, pos, &word);
        int res;
        while (pos > 0){

            //normalize the word
            res = NormalizeWord(word);

            //if it is a valid word
            if (pos > 0 && res != 0){
                fprintf(f, "%s", word);
                fprintf(f, "\n");

                // for the current document, if a new word is encountered
                // place it in a queue with count 1 and document_t
                queue_of_documents_t *temp;
                if ((temp = hsearch(index, &document_queue_search, word, strlen(word))) == NULL){
                    queue_of_documents_t *q_docs = malloc(sizeof(queue_of_documents_t));
                    // note: do we need to malloc here or can we just assign? - because word gets overwritten??
                    q_docs->word = word;
                    q_docs->qp = qopen();
                    // put the current id and count 1 in the queue
                    document_t *dp = malloc(sizeof(document_t));
                    dp->id = idx;
                    dp->count = 1;
                    qput(q_docs->qp, dp);
                    hput(index, q_docs, word, strlen(word));
                }
                else {
                    // find the document entry in the queue - if it's not there, then add a new document_t entry to the queue
                    queue_of_documents_t *q_docs = temp;
                    document_t *temp_doc;
                    if ((temp_doc = qsearch(q_docs->qp, &document_word_search, &idx)) == NULL) {
                        document_t *dp = malloc(sizeof(document_t));
                        dp->id = idx;
                        dp->count = 1;
                        qput(q_docs->qp, dp);
                    }
                    else {
                        temp_doc->count++;
                    }
                    // free the word because it wasn't added to a queue_of_documents structure
                    free(word);
                }
            }
            pos = webpage_getNextWord(page, pos, &word);
        }
        webpage_delete(page);
        free(word);
        idx++;
        page = pageload(idx, dir);
    }
    //save the index
    printf("saving the index\n");
    indexsave(index, indexnm);

    // need to free the queues
    happly(index, &free_queues);

    hclose(index);
    fclose(f);
    return (EXIT_SUCCESS);
}