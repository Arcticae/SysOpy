//
// Created by timelock on 13.03.18.
//

#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*static_array[1000000];

struct array_blocks* create_array(int blocks,int is_static) { //static=0 for dynamic array, static =1 for static array allocation

    if(blocks<=0)return NULL;

    array_blocks* new_array= malloc(sizeof(array_blocks));
    new_array->block_number=blocks-1;       //because of indexing

    if(is_static) {
        new_array->is_static=1;      //will be important when writing other functions
        new_array->array=static_array;
    }
    else {
        new_array->is_static=0;
        new_array->array=(char**)malloc(blocks*sizeof(char*));
    }



    return new_array;

}

void delete_array(struct array_blocks*array) {
    if(array == NULL) return;
    int i;
    for(i=0;i<array->block_number;i++) {
        if(array->array[i]!=NULL) {
            free(array->array[i]);
        }
    }
}

void delete_block_at(struct array_blocks*array,int index){
    if(array->block_number<index || array == NULL)return;
    if(array->array[index]!=NULL) {
        free(array->array[index]);
        array->array[index]=NULL;
    }
}

void add_block_at(struct array_blocks*array,int index.char*blk){

    if(array==NULL || array->array_blocks<index || index < 0) return;

    array->array[index]=(char*)malloc(sizeof(char)*strlen(blk));
    strcpy(array->array[index],blk);
}

int get_block_value(char*blk){

    if(blk==NULL)return 0;
    int i;
    int value=0;
    for(i=0;i<strlen(blk);i++)
    {
        value+=(int)blk[i];
    }
    return value;
}

char* get_nearest_block_bysum(struct array_blocks*array,int blocknumber){

    if(array== null || blocknumber<0 || blocknumber>array->block_number){
        return null;
    }

    int blocknumber_sum=get_block_value(array->array[blocknumber]);
    int i;
    int minimum=MAX_INT;
    char*nearest;

    for(i=0;i<array->block_number;i++)
    {
        char*tmp=array->array[i];

        int current=abs(get_block_value(tmp)-blocknumber_sum);
        if(current < minimum) {
            nearest=tmp;
            minimum=current;
        }
    }
    return nearest;
}


