
#include <stdio.h>
#include <stdlib.h>
#include "webpage.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./print_urls <base_url>\n");
        exit(EXIT_FAILURE);
    }
    char *url = argv[1];
    webpage_t *pg = webpage_new(url, 0, NULL);
    bool res = webpage_fetch(pg);
    if (!res) {
        printf("fetch failed\n");
        exit(EXIT_FAILURE);
    }
    int pos = 0;
    char *q_url;
    while ((pos = webpage_getNextURL(pg, pos, &q_url)) > 0) {
        if (IsInternalURL(q_url)) {
            printf("%s\n", q_url);
            free(q_url);
        }
    }
    webpage_delete(pg);
    exit(EXIT_SUCCESS);
}