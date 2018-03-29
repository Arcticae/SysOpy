//
// Created by timelock on 29.03.18.
//

#include <stdlib.h>

int main(){
    int size=1000000000;
    int i;
    char*table=malloc(size*sizeof(char));
    for(i=0;i<size;i++){
        table[i]='a';
    }
    free(table);

    return 0;
}