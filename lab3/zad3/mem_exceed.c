//
// Created by timelock on 29.03.18.
//

#include <stdlib.h>
#include <stdio.h>

int main(){
    long int size=1024*1024*10;
    int i,j;
    char *table=NULL;
    for(i=1;i<10;i++) {
        table = calloc((size_t) size*i, sizeof(char));
        printf("Allocated %ldMiB of memory\n", i*size / (1024 * 1024));

        for (j = 0; j < size; j++) {
            table[j] = 'a';
        }
        free(table);
    }


    return 0;
}