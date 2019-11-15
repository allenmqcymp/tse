/*
 * tests takong stuff out of the lqueue
 *  - puts 5 person into the lqueue
 *  - searches for one of them by age
 * module 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "time.h"
#include "lqueue.h"

#define MAXNM 128

typedef struct person {
    char name[MAXNM];
    int age;
    double rate;
} person_t;

bool search_fun(void *element, const void* search){
    person_t *e = (person_t *)element;
    person_t *s = (person_t *)search;
    if (e->age == s->age){
        return true;
    }
    return false;
}

int main(void) {

    lqueue_t *qp = lqopen();
    person_t person1 = {"stjepan", 21, 5};
    person_t person2 = {"allen", 22, 5};
    person_t person3 = {"joe", 23, 5};
    person_t person4 = {"selim", 24, 5};
    person_t person5 = {"musab", 25, 5};
    
    if (lqput(qp, (void *)&person1) != 0){
        exit(EXIT_FAILURE);
    }
    if (lqput(qp, (void *)&person2) != 0){
        exit(EXIT_FAILURE);
    }
    if (lqput(qp, (void *)&person3) != 0){
        exit(EXIT_FAILURE);
    }
    if (lqput(qp, (void *)&person4) != 0){
        exit(EXIT_FAILURE);
    }
    if (lqput(qp, (void *)&person5) != 0){
        exit(EXIT_FAILURE);
    }
    
    person_t search = {"aaaa", 23, 5};
    person_t *joe = (person_t *)lqsearch(qp, &search_fun, &search);
    if (joe->age != 23){
        exit(EXIT_FAILURE);
    }
    printf("success");
    lqclose(qp);
    exit(EXIT_SUCCESS);
}
