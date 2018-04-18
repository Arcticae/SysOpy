//
// Created by timelock on 18.04.18.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char**argv){

    if(argc<2){
        fprintf(stderr,"Commandline arguments are not sufficient\n");
        exit(EXIT_FAILURE);
    }
    if(mkfifo(argv[1], S_IWUSR |S_IWGRP | S_IRGRP | S_IRUSR)==-1){      //write+read permissions for group and user
        perror("Creating fifo file didn't go well");
        exit(EXIT_FAILURE);
    }
    FILE* fifoptr=fopen(argv[1],"r");       //need only to listen to fifo
    if(fifoptr==NULL){
        perror("Opening fifo file went wrong");
        exit(EXIT_FAILURE);
    }
    char buffer[PIPE_BUF];
    int amount=0;
    while(fgets(buffer,PIPE_BUF,fifoptr)){
        printf("Master: I have received a buffer from slave\n%s",buffer);
        amount++;
    }
    fprintf(stderr,"Master got %d buffers in total\n",amount);
    fclose(fifoptr);
    unlink(argv[1]);
    return 0;
}