//
// Created by timelock on 13.03.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "library.h"

char* get_random_block(int length){
    if(length<1)return NULL;
    char*base="0123456789ZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY";
    int base_len=strlen(base);
    char* swieradstring=(char*)malloc(length*sizeof(char));

    int i;
    for(i=0;i<length-1;i++){
        swieradstring[i]=base[rand() %base_len];

    }
    swieradstring[length-1]='\0';
    return swieradstring;
}

void randomize_array(struct array_blocks* array, int width){

    if(array==NULL)return;
    if(width<0)return;
    int i;
    for(i=0;i<array->block_number;i++) {
        char*block=get_random_block(width);
        add_block_at(array,i,block);
    }
}
void add_n_blocks(struct array_blocks*array, int n,int max_width){
    if(array==NULL)return;
    if(n-1>array->block_number)return;//too many blocks for the sizeofarray
    int i;
    for(i=0;i<n;i++){
        int len=rand()% max_width + 1;
        char*random_blk=get_random_block(len);
        add_block_at(array,i,random_blk);
    }
}

void delete_n_blocks(struct array_blocks* array,int n){

    if(n-1>array->block_number)return;
    int i;
    for(i=0;i<n;i++){
        delete_block_at(array,i);
    }
}

void add_and_delete_n_blocks(struct array_blocks*array,int n,int width){

    add_n_blocks(array,n,width);
    delete_n_blocks(array,n);
}

int main()
{
    printf("hello world!\n");
    struct array_blocks *newarray=create_array(200,1);
    printf("cos");


}
