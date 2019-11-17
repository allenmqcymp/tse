#pragma once
/*
 * lhash.h -- A generic lhash table implementation, allowing arbitrary
 * key structures.
 *
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "queue.h"

typedef void lhashtable_t;	/* representation of a lhashtable lhidden */

/* lhopen -- opens a lhash table with initial size lhsize */
lhashtable_t *lhopen(uint32_t lhsize);

/* lhclose -- closes a lhash table */
void lhclose(lhashtable_t *lhtp);

/* lhput -- puts an entry into a lhash table under designated key 
 * returns 0 for success; non-zero otherwise
 */
int32_t lhput(lhashtable_t *lhtp, void *ep, const char *key, int keylen);

/* lhapply -- applies a function to every entry in lhash table */
void lhapply(lhashtable_t *lhtp, void (*fn)(void* ep));

/* lhsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *lhsearch(lhashtable_t *lhtp, 
	      bool (*searchfn)(void* elementp, const void* searchkeyp), 
	      const char *key, 
	      int32_t keylen);

/* lhremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *lhremove(lhashtable_t *lhtp, 
	      bool (*searchfn)(void* elementp, const void* searchkeyp), 
	      const char *key, 
	      int32_t keylen);

bool lhsearch_add(lhashtable_t *lhtp, 
        bool (*searchfn)(void* elementp,const void* keyp), 
        const char* key, 
        int32_t keylen);