/* indexer.c ---
 * 
 * 
 * Author: Stjepan Vrbic, Allen Ma
 * 
 * HTLM indexer
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "pageio.h"
#include "webpage.h"
#include "hash.h"

//word structure
typedef struct word_count{
    char* word;
    int count;
}word_count_t;

int sum = 0;

void sum_counts(void *ep){
    word_count_t *temp = (word_count_t*)ep;
    sum += temp->count;
}

void free_word(void *ep) {
    word_count_t *temp = (word_count_t *) ep;
    free(temp->word);
    free(temp);
}

bool word_search(void *ep, const void *sp){
    word_count_t *h_word = (word_count_t*)ep;
    char *s_word = (char*)sp;
    if (strcmp(h_word->word, s_word) == 0){
        return true;
    }else{
        return false;
    }
}

//returns non 0 if the word is alphabetic and >3, returns 0 if the word is rejected
int NormalizeWord(char *word){
    bool alpha = true;
    for (int i = 0; i<strlen(word); i++){
        if (isalpha(word[i]) == 0){
            alpha = false;
            break;
        }else{
            word[i] = tolower(word[i]);
        }
    }
    if (strlen(word) < 3 || !alpha){
        free(word);
        return 0;
    }

    return 1;
}

int main(void){
    
    int id = 1;
    char *dir = "../pages/";
    webpage_t *page = pageload(id, dir);
    int pos = 0;
    char *word = NULL;
    pos = webpage_getNextWord(page, pos, &word);

    //make the file and check if everything went right
    FILE *f = fopen("output_file", "w");
    if (f == NULL) {
        printf("failed to open file %s\n", "output_file");
		printf("Error %d \n", errno);
        return -1;
    }

    //make a hashtable to index the occurences of each word
    hashtable_t *index = hopen(1000);

    int res;
    while (pos > 0){

        //normalize the word
        res = NormalizeWord(word);

        //if it is a valid word
        if (pos > 0 && res != 0){
            fprintf(f, "%s", word);
            fprintf(f, "\n");

            //put it in the hash table if it is a new word, increment count if it already exists
            word_count_t *temp;
            if ((temp = hsearch(index, &word_search, word, strlen(word))) == NULL){
                word_count_t *counter = malloc(sizeof(word_count_t));
                counter->word = word;
                counter->count = 1;
                hput(index, counter, counter->word, strlen(counter->word));
            }else{
                temp->count++;
                free(word);
            }
        }
        pos = webpage_getNextWord(page, pos, &word);
    }
    happly(index, &sum_counts);
    printf("the count is: %d\n", sum);
    webpage_delete(page);
    happly(index, &free_word);
    hclose(index);
    fclose(f);
    free(word);
    return (EXIT_SUCCESS);
}
