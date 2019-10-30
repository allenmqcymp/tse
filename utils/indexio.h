#pragma once
/* 
 * indexio.h
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
#include "hash.h"

/*
 * indexsave() - saves index - a hashtable of queue_of_documents
 * a queue_of_documents is an abstract container struct containing a word, and a queue
 * inside the queue are document_t structs, each of which contains a document id, and a count associated with how many
 * times the word appears in the document
 * 
 * saves the index to file specified by dirnm in the format
 * <word> <docID1> <count1> <docID2> <count2> ....<docIDN> <countN>, where each word is on its own line
 * 
 * index must be freed after this function finishes
 * 
 * returns: 0 on success and -1 on failure
 */
int32_t indexsave(hashtable_t *index, char *dirnm);

/* 
 * indexload() - takes in a given filename that contains an parsable index file, in the form of
 * <word> <docID1> <count1> <docID2> <count2> ....<docIDN> <countN>
 * where each word is on its own line
 * 
 * returns new hashtable that user is responsible for freeing later
 * if it fails for some reason, then it returns NULL
 */
hashtable_t *indexload(char *dirnm);
