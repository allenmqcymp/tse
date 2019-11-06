/* 
 * test indexio --- Implements the functions specified in pageio.h
 * 
 * Author: Allen Ma, Stjepan Vrbic
 * Created: Tuesday Oct 29
 * Version: 1.0
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <webpage.h>
#include <pageio.h>
#include <errno.h>

#include "indexio.h"
#include "hash.h"
#include "queue.h"

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

int main(int argc, char *argv[]) {
    char *fname = "./test_indices/indexer1";
    printf("running test indexio on %s\n", fname);
    hashtable_t *h = indexload(fname);
    // save the index using indexsave
    indexsave(h, "./test_indices/indexer1_saved");
    // check that he two files are the same
    system("diff ./test_indices/indexer1 ./test_indices/indexer1_saved");
    // free everything
    happly(h, &free_queues);
    hclose(h);
    return 0;
}
