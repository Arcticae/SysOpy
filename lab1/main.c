//
// Created by timelock on 13.03.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/resource.h>
#include "library.h"

char* get_random_block(int length){
    if(length<1)return NULL;
    char*base="0123456789ZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY";
    int base_len=(int) strlen(base);
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
void add_n_blocks(struct array_blocks*array, int n,int width){
    if(array==NULL)return;
    if(n-1>array->block_number)return;//too many blocks for the sizeofarray
    int i;
    for(i=0;i<n;i++){
        char*random_blk=get_random_block(width);
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
void print_array(struct array_blocks* array)
{
    int i;
    for(i=0;i<array->block_number;i++){


            if(array->array[i]!=NULL)
                printf("%s\n",array->array[i]);
            else printf("NULL\n");
    }
    printf("\n");
}
void do_stuff(char*operation,int n,int block_width,struct array_blocks*array){

    if(strcmp(operation,"search_element")==0){
        get_nearest_block_bysum(array,n);

    }
    if(strcmp(operation,"remove")==0){
        delete_n_blocks(array,n);
    }
    if(strcmp(operation,"add")==0){
        add_n_blocks(array,n,block_width);
    }
    if(strcmp(operation,"remove_and_add")==0){
        add_and_delete_n_blocks(array,n,block_width);
    }
}

int main(int argc,char**argv)
{
   if(argc<4){
        printf("Please give the program proper number of arguments. \n"
               "(1) Mode of allocation - 1 for static 0 for dynamic.\n"
               "(2) Blocksize - in range of integer.\n"
               "(3) Number of blocks available in the array.\n"
               "(4) Optional 2 commands from the list as given : search_element <arg> , remove <arg> , add <arg> , remove_and_add <arg>");
            return 1;
    }
    srand((unsigned int)time(NULL));

    int is_static=(int)strtol(argv[1],NULL,10);    //no strtoi in stdlib.h?
    int blocksize=(int)strtol(argv[2],NULL,10);
    int blocknumber=(int)strtol(argv[3],NULL,10);

    if(is_static!=0 && is_static!=1){
        printf("Wrong allocation mode argument please give me 0 or 1 (1 for static)!\n");
        return 1;
    }

    char*command_1,*command_2;
    int arg1,arg2;

    if(argc>=5)
    {
        command_1=argv[4];
    }
    if(argc>=6)
    {
        arg1=(int)strtol(argv[5],NULL,10);
    }
    if(argc>=7)
    {
        command_2=argv[6];
    }
    if(argc>=8)
    {
        arg2=(int)strtol(argv[7],NULL,10);
    }

    struct timeval* user_times=malloc(6*sizeof(struct timeval));
    struct timeval* sys_times=malloc(6*sizeof(struct timeval));
    struct timeval** real_times=malloc(6*sizeof(struct timeval*));

    for(int i=0;i<6;i++){

        real_times[i]=(struct timeval*)malloc(sizeof(struct timeval*));
    }


    struct array_blocks * array;
    struct rusage* before=(struct rusage*)malloc(sizeof(struct rusage));
    struct rusage* after=(struct rusage*)malloc(sizeof(struct rusage));

    //start measure
    gettimeofday(real_times[0],NULL);
    getrusage(RUSAGE_SELF,before);

    array=create_array(blocknumber,is_static);
    randomize_array(array,blocksize);


    gettimeofday(real_times[1],NULL);
    getrusage(RUSAGE_SELF,after);

    sys_times[0]=before->ru_stime;
    sys_times[1]=after->ru_stime;

    user_times[0]=before->ru_utime;
    user_times[1]=after->ru_utime;
    //end of measure

    printf("\nCreating and filling table time with dimensions of %d blocks, and %d width of a block\n",blocknumber,blocksize);
    printf("Realtime: %ld.%06lds\n",real_times[1]->tv_sec-real_times[0]->tv_sec,real_times[1]->tv_usec-real_times[0]->tv_usec);
    printf("System Time: %ld.%06lds\n",sys_times[1].tv_sec-sys_times[0].tv_sec,sys_times[1].tv_usec-sys_times[0].tv_usec);
    printf("User Time: %ld.%06lds\n\n",user_times[1].tv_sec-user_times[0].tv_sec,user_times[1].tv_usec-user_times[0].tv_usec);

    if(argc>=5)
    {
        gettimeofday(real_times[2],NULL);
        getrusage(RUSAGE_SELF,before);

        do_stuff(command_1,arg1,blocksize,array);

        gettimeofday(real_times[3],NULL);
        getrusage(RUSAGE_SELF,after);

        sys_times[2]=before->ru_stime;
        sys_times[3]=after->ru_stime;

        user_times[2]=before->ru_utime;
        user_times[3]=after->ru_utime;

        printf("%s: with arg %d\n",command_1,arg1);
        printf("Realtime: %ld.%06lds\n",real_times[3]->tv_sec-real_times[2]->tv_sec,real_times[3]->tv_usec-real_times[2]->tv_usec);
        printf("System Time: %ld.%06lds\n",sys_times[3].tv_sec-sys_times[2].tv_sec,sys_times[3].tv_usec-sys_times[2].tv_usec);
        printf("User Time: %ld.%06lds\n\n",user_times[3].tv_sec-user_times[2].tv_sec,user_times[3].tv_usec-user_times[2].tv_usec);


    }

    if(argc>=7)
    {
        gettimeofday(real_times[4],NULL);
        getrusage(RUSAGE_SELF,before);

        do_stuff(command_2,arg2,blocksize,array);

        gettimeofday(real_times[5],NULL);
        getrusage(RUSAGE_SELF,after);

        sys_times[4]=before->ru_stime;
        sys_times[5]=after->ru_stime;

        user_times[4]=before->ru_utime;
        user_times[5]=after->ru_utime;

        printf("%s: with arg %d\n",command_2,arg2);
        printf("Realtime: %ld.%06lds\n",real_times[5]->tv_sec-real_times[4]->tv_sec,real_times[5]->tv_usec-real_times[4]->tv_usec);
        printf("System Time: %ld.%06lds\n",sys_times[5].tv_sec-sys_times[4].tv_sec,sys_times[5].tv_usec-sys_times[4].tv_usec);
        printf("User Time: %ld.%06lds\n",user_times[5].tv_sec-user_times[4].tv_sec,user_times[5].tv_usec-user_times[4].tv_usec);


    }


}
