/*
 * tests taking stuff out of the lqueue
 *  - tests qapply for an empty lqueue
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

void apply(void *element){
    person_t *e = (person_t *)element;
    e->age = e->age + 1;
}

int main(void) {

    lqueue_t *qp = lqopen();
    lqapply(qp, &apply);
    
    lqclose(qp);
    exit(EXIT_SUCCESS);
}
