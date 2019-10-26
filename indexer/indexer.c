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

#include "pageio.h"
#include "webpage.h"

int main(void){
    
    int id = 1;
    char *dir = "../pages/";
    webpage_t *page = pageload(id, dir);
    

    return (EXIT_SUCCESS);
}
