#pragma once
/* 
 * lqueue.h -- public interface to the locked queue module
 */
#include <stdint.h>
#include <stdbool.h>

/* the queue representation is hidden from users of the module */
typedef void lqueue_t;		

/* create an empty queue */
lqueue_t* lqopen(void);        

/* deallocate a queue, frees everything in it */
void lqclose(lqueue_t *qp);   

/* put element at the end of the queue
 * returns 0 is successful; nonzero otherwise 
 */
int32_t lqput(lqueue_t *qp, void *elementp); 

/* get the first first element from queue, removing it from the queue */
void* lqget(lqueue_t *qp);

/* apply a function to every element of the queue */
void lqapply(lqueue_t *qp, void (*fn)(void* elementp));

/* search a queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- elementp - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be 
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void* lqsearch(lqueue_t *qp, 
							bool (*searchfn)(void* elementp,const void* keyp),
							const void* skeyp);

/* search a queue using a supplied boolean function (as in qsearch),
 * removes the element from the queue and returns a pointer to it or
 * NULL if not found
 */
void* lqremove(lqueue_t *qp,
							bool (*searchfn)(void* elementp,const void* keyp),
							const void* skeyp);

/* concatenatenates elements of q2 into q1
 * q2 is dealocated, closed, and unusable upon completion 
 */
void lqconcat(lqueue_t *q1p, lqueue_t *q2p);

/* peek the queue - returns pointer to the head of the queue. returns NULL if queue is empty */
void *lqpeek(lqueue_t *qp);

/*
 * Performs a search, and if successful, performs the action, and returns true, to indicate that
 * the action was performed
 * Otherwise, return false to indicate the action was not performed
 */
bool lqsearch_action(lqueue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp, void (*fn)(void *ep));
