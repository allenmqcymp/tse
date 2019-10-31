/* 
 * indexio.c
 * 
 * Author: Allen Ma, Stjepan Vrbic
 * Created: Tuesday 29 Oct
 * Version: 1.0
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <webpage.h>
#include <errno.h>
#include "hash.h"
#include "queue.h"

static hashtable_t *index;
static queue_t *temp_queue;

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

void get_word_and_docs(void *ep) {
    queue_of_documents_t *q_dp = (queue_of_documents_t *) ep;
    // add the queue to temp_queue
    qput(temp_queue, q_dp);
}



int32_t indexsave(hashtable_t *h_index, char *fname) {
    FILE *f = fopen(fname, "w");
    if (f == NULL) {
        printf("failed to open file %s\n", f);
		printf("Error %d \n", errno);
        return -1;
    }

    // check if it's possible to write to the directory
	if (access(f, W_OK) != 0) {
		printf("cannot access %s\n", f);
		return -1;
	}

    // make a temporary queue
    temp_queue = qopen();

    // for each word
    happly(h_index, &get_word_and_docs);

    int init_size = 64;

    queue_of_documents_t *q_dp;
    while ((q_dp = qget(temp_queue)) != NULL) {
        // get the word from the queue
        char *word = q_dp->word;

        char *write_buf = malloc(strlen(word) + 1 + init_size);
        sprintf(write_buf, )
        document_t *doc_t;
        // word a word, then a space to the buffer
        while ((doc_t = qget(q_dp->qp)) != NULL) {
            //
        }
    }


    fclose(f);

    return 0;

}


hashtable_t *indexload(char *fname) {

    // somehow need to get all the words into a queue
    // but I don't want to use a static queue approach
    // is there an alternative

    FILE *f = fopen(fname, "r");
    if (f == NULL) {
        printf("indexload: could not open file\n");
    }

    printf("HALLO!\n");
    char line[255];
    const char s[2] = " ";

    // instantiate an index hashtable
    index = hopen(1000);
    
    while(fgets(line, sizeof(line), f) != NULL) {
        
        char *token;
        char *word;
        if (line[0] == '\r' || line[0] == '\n') {
            continue;
        }
        
        // parse one line of the output

        // remove newline at the end of the output
        line[strcspn(line, "\n")] = 0;

        // the word is the first token
        token = strtok(line, s);
        word = token;
        printf( "word is %s\n", word );

        // put the word in the hashtable
        queue_of_documents_t *q_docs = malloc(sizeof(queue_of_documents_t));
        q_docs->qp = qopen();
        q_docs->word = malloc(strlen(word) + 1);
        strcpy(q_docs->word, word);
        hput(index, q_docs, word, sizeof(word));

        // parse the rest of them
        char *docid;
        char *count;
        while( token != NULL ) {
            token = strtok(NULL, s);
            docid = token;
            token = strtok(NULL, s);

            if (token == NULL) {
                break;
            }
            count = token;
            //add it to the hashtable
            document_t *dp = malloc(sizeof(document_t));
            dp->id = atoi(docid);
            dp->count = atoi(count);
            qput(q_docs->qp, dp);

            if (token == NULL) {
                printf("odd number of matches\n");
                return NULL;
            }
        }
    }

    fclose(f);
    return index;
}
