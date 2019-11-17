/*
 * tests opening and closing of the lhashtable
 * module 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "lhash.h"
#include "list.h"

int main(void) {
    lhashtable_t *htp2 = lhopen(500);

    if (htp2 == NULL) {
        exit(EXIT_FAILURE);
    }

    lhclose(htp2);

    exit(EXIT_SUCCESS);
}
