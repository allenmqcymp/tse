/*
 * tests taking stuff out of the lqueue
 * performs 3 tests:
 *  - puts 5 person into the lqueue
 *  - takes all of them out
 *  - tests removing from empty lqueue
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

int main(void) {

    lqueue_t *qp = lqopen();
    person_t person1 = {"stjepan", 21, 1};
    person_t person2 = {"allen", 21, 2};
    person_t person3 = {"allen", 21, 3};
    person_t person4 = {"allen", 21, 4};
    person_t person5 = {"allen", 21, 5};
    lqput(qp, (void *)&person1);
    lqput(qp, (void *)&person2);
    lqput(qp, (void *)&person3);
    lqput(qp, (void *)&person4);
    lqput(qp, (void *)&person5);

    person_t *p = (person_t *)lqget(qp);
    if (p->rate != 1){
        exit(EXIT_FAILURE);
    }
    p = (person_t *)lqget(qp);
    if (p->rate != 2){
        exit(EXIT_FAILURE);
    }
    p = (person_t *)lqget(qp);
    if (p->rate != 3){
        exit(EXIT_FAILURE);
    }
    p = (person_t *)lqget(qp);
    if (p->rate != 4){
        exit(EXIT_FAILURE);
    }
    p = (person_t *)lqget(qp);
    if (p->rate != 5){
        exit(EXIT_FAILURE);
    }

    lqget(qp);
    
    lqclose(qp);
    printf("success");
    exit(EXIT_SUCCESS);
}
