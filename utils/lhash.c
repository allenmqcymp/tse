/*
 * lhash.h -- A generic lhash table implementation, allowing arbitrary
 * key structures.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include "hash.h"
#include "lqueue.h"

typedef hashtable_t lhashtable_t;	/* representation of a lhashtable lhidden */

pthread_mutex_t mutexh = PTHREAD_MUTEX_INITIALIZER;

/* lhopen -- opens a lhash table with initial size lhsize */
lhashtable_t *lhopen(uint32_t lhsize) {
    return hopen(lhsize);
}

/* lhclose -- closes a lhash table */
void lhclose(lhashtable_t *lhtp) {
    pthread_mutex_lock(&mutexh);
    hclose(lhtp);
    pthread_mutex_unlock(&mutexh);
}

/* lhput -- puts an entry into a lhash table under designated key 
 * returns 0 for success; non-zero otherwise
 */
int32_t lhput(lhashtable_t *lhtp, void *ep, const char *key, int keylen) {
    int ret_val = -1;
    pthread_mutex_lock(&mutexh);
    ret_val = hput(lhtp, ep, key, keylen);
    pthread_mutex_unlock(&mutexh);
    return ret_val;
}

/* lhapply -- applies a function to every entry in lhash table */
void lhapply(lhashtable_t *lhtp, void (*fn)(void* ep)) {
    pthread_mutex_lock(&mutexh);
    happly(lhtp, fn);
    pthread_mutex_unlock(&mutexh);
}

/* lhsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *lhsearch(lhashtable_t *lhtp, 
	      bool (*searchfn)(void* elementp, const void* searchkeyp), 
	      const char *key, 
	      int32_t keylen) {
    
    void *r = NULL;
    pthread_mutex_lock(&mutexh);
    r = hsearch(lhtp, searchfn, key, keylen);
    pthread_mutex_unlock(&mutexh);
    return r;
}

/* lhremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *lhremove(lhashtable_t *lhtp, 
	      bool (*searchfn)(void* elementp, const void* searchkeyp), 
	      const char *key, 
	      int32_t keylen) {
    void *r = NULL;
    pthread_mutex_lock(&mutexh);
    r = hremove(lhtp, searchfn, key, keylen);
    pthread_mutex_unlock(&mutexh);
    return r;
}

/*
 * Performs a search, and if found, 
 * places the found item into the hashtable, then performs a function denoted by fn
 */
bool lhsearch_add(lhashtable_t *lhtp, 
        bool (*searchfn)(void* elementp,const void* keyp), 
        const char* key, 
        int32_t keylen) {

    // found it true by default - since if it's true, we don't have to do anything
    bool added = false;
    void *r = NULL;
    pthread_mutex_lock(&mutexh);
    r = hsearch(lhtp, searchfn, key, keylen);
    if (r == NULL) {
        // place url in the hashtable
        hput(lhtp, (void *) key, key, keylen);
        added = true;
    }
    pthread_mutex_unlock(&mutexh);
    return added;
}
