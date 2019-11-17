#include <stdio.h>
#include <stdlib.h>

void foo(int *i) {
    int k = 0;
    i = &k;
}

int main(void) {
    int j = 0;
    int *i;
    foo(&j);
    // compare up or down

    //compare the address of j and i
    return 0;
}

