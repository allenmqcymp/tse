/* 
 * Allen Ma, Stjepan Vrbic
 * lqueue.c -- implements a generic locked queue
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "queue.h"

typedef queue_t lqueue_t;

pthread_mutex_t mutexq = PTHREAD_MUTEX_INITIALIZER;

lqueue_t* lqopen(void) {
    return qopen();
}

/* deallocate a queue, frees everything in it */
void lqclose(lqueue_t *qp) {
    pthread_mutex_lock(&mutexq);
    qclose(qp);
    pthread_mutex_unlock(&mutexq);
}

int lqlen(lqueue_t *qp) {
    pthread_mutex_lock(&mutexq);
    int len = qlen(qp);
    pthread_mutex_unlock(&mutexq);
    return len;
}


/* put element at the end of the queue
 * returns 0 is successful; nonzero otherwise 
 */
int32_t lqput(lqueue_t *qp, void *elementp) {
    int ret_val = -1;
    pthread_mutex_lock(&mutexq);
    ret_val = qput(qp, elementp);
    pthread_mutex_unlock(&mutexq);
    return ret_val;
}

/* get the first first element from queue, removing it from the queue */
void* lqget(lqueue_t *qp) {
    void *r = NULL;
    pthread_mutex_lock(&mutexq);
    r = qget(qp);
    pthread_mutex_unlock(&mutexq);
    return r;
}

/* apply a function to every element of the queue */
void lqapply(lqueue_t *qp, void (*fn)(void* elementp)) {
    pthread_mutex_lock(&mutexq);
    qapply(qp, fn);
    pthread_mutex_unlock(&mutexq);
}

/* search a queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- elementp - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be 
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void* lqsearch(lqueue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
    void *r = NULL;
    pthread_mutex_lock(&mutexq);
    r = qsearch(qp, searchfn, skeyp);
    pthread_mutex_unlock(&mutexq);
    return r;
}

/* search a queue using a supplied boolean function (as in qsearch),
 * removes the element from the queue and returns a pointer to it or
 * NULL if not found
 */
void* lqremove(lqueue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
    void *r = NULL;
    pthread_mutex_lock(&mutexq);
    r = qremove(qp, searchfn, skeyp);
    pthread_mutex_unlock(&mutexq);
    return r;
}

/*
 * Performs a search, and if successful, performs the action, and returns true, to indicate that
 * the action was performed
 * Otherwise, return false to indicate the action was not performed
 */
bool lqsearch_action(lqueue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp, void (*fn)(void *ep)) {
    bool success = false;
    void *r = NULL;
    pthread_mutex_lock(&mutexq);
    r = qsearch(qp, searchfn, skeyp);
    if (r != NULL) {
        // perform the action
        fn(r);
        success = true;
    }
    pthread_mutex_unlock(&mutexq);
    return success;
}

/* concatenatenates elements of q2 into q1
 * q2 is dealocated, closed, and unusable upon completion 
 */
void lqconcat(lqueue_t *q1p, lqueue_t *q2p) {
    pthread_mutex_lock(&mutexq);
    qconcat(q1p, q2p);
    pthread_mutex_unlock(&mutexq);
}