/*
 * tests putting stuff into the lhashtable
 * performs 3 tests:
 *  - puts one person_t, hashing by name
 *  - puts 50 person_t, hashing by age
 *  - puts 5 person_t, with the same name, hashing by name
 * module 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "time.h"
#include "lhash.h"

#define MAXNM 128

typedef struct person {
    char name[MAXNM];
    int age;
    double rate;
} person_t;

person_t make_person(const char *name, int age, double rate) {
    person_t p;
    strcpy(p.name, name);
    p.age = age;
    p.rate = rate;
    return p;
}


/*
 * Allocates memory for person_t
 */
int test_put_one(lhashtable_t *htp) {
    person_t p = make_person("allen", 21, 21.21);
    person_t p3 = make_person("sam", 21, 21.21);
    int32_t res =  lhput(htp, (void *) &p, "allen", sizeof("allen"));
    if (res != 0) {
	return 1;
    }
    int32_t res3 = lhput(htp, (void *) &p3, "sam", sizeof("sam"));
    if (res3 != 0) {
	return 1; 
    }
    return 0;
}


int test_put_two(lhashtable_t *htp) {
    char *arr[5];
    arr[0] = "joey";
    arr[1] = "askjfhklajsdhfkjhklgajghlajshgkljhasgkjaskdjhfak";
    arr[2] = "dmitry";
    arr[3] = "laurent";
    arr[4] = "giovanni";
    for (int i = 0; i < 50; i++) {
        char *randname = arr[rand() % 5];
        int randage = rand() % 100;
        person_t p = make_person(randname, randage, 10.00);

        int32_t res = lhput(htp, (void *) &p, (char *) &randage, sizeof(randage));
        if (res != 0) {
            printf("lhput failed\n");
            return 1;
        }
    }
    return 0;
}

int test_put_three(lhashtable_t *htp) {
    person_t pp1 = make_person("allen", 21, 10.00);
    person_t pp2 = make_person("allen", 22, 10.00);
    person_t pp3 = make_person("allen", 21, 10.00);
    person_t pp4 = make_person("allen", 21, 12.00);
    person_t pp5 = make_person("allen", 50, 40.00);

    bool ok = true;

    if (lhput(htp, (void *) &pp1, "allen", sizeof("allen")) != 0) {
        printf("lput on person1 failed\n");
        ok = false;
    }

    if (lhput(htp, (void *) &pp2, "allen", sizeof("allen")) != 0) {
        printf("lput on person2 failed\n");
        ok = false;
    }

    if (lhput(htp, (void *) &pp3, "allen", sizeof("allen")) != 0) {
        printf("lput on person3 failed\n");
        ok = false;
    }

    if (lhput(htp, (void *) &pp4, "allen", sizeof("allen")) != 0) {
        printf("lput on person4 failed\n");
        ok = false;
    }

    if (lhput(htp, (void *) &pp5, "allen", sizeof("allen")) != 0) {
        printf("lput on person5 failed\n");
        ok = false;
    }


    if (!ok) {
        return 1;
    }
    return 0;
}

int main(void) {

    // this also implicitly tests that there can be more than one lhashtable in existence
    // at the same time
    lhashtable_t *htp1 = lhopen(4);
    lhashtable_t *htp2 = lhopen(100);
    lhashtable_t *htp3 = lhopen(500);

    srand(time(NULL));

    if (test_put_one(htp1) != 0) {
    	printf("failed test put one\n");
    	exit(EXIT_FAILURE);
    }

    if (test_put_two(htp2) != 0) {
        printf("failed test put two\n");
        exit(EXIT_FAILURE);
    }

    if (test_put_three(htp3) != 0) {
        printf("failed test put three\n");
        exit(EXIT_FAILURE);
    }

    lhclose(htp1);
    lhclose(htp2);
    lhclose(htp3);

    exit(EXIT_SUCCESS);
}
