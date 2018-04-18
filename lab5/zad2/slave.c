//
// Created by timelock on 18.04.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#define date_size 33

int main(int argc, char**argv){
    if(argc<3){
        printf("Wrong arguments given to the program, please try agen\n");
        exit(EXIT_FAILURE);
    }
    FILE*fifoptr=fopen(argv[1],"w");
    if(fifoptr==NULL){
        perror("Problem with opening fifo file");
        exit(EXIT_SUCCESS);
    }
    int N=(int)strtol(argv[2],NULL,10);
    if(N<=0){
        fprintf(stderr,"Not correct format of times the date has to be sent!\n");
        exit(EXIT_FAILURE);
    }
    srand((unsigned int)getpid()%100+1);
    printf("Child: My pid is: %d\n",getpid());
    int i;
    char date_buffer[date_size];
    char pipe_output[PIPE_BUF];

    for(i=0;i<N;i++){
        FILE*date_pipe=popen("date","r");
        if(date_pipe==NULL){
            perror("Something wrong with getting date");
            exit(EXIT_FAILURE);
        }
        fgets(date_buffer,date_size,date_pipe);
        fclose(date_pipe);
        sprintf(pipe_output,"My pid is %d and my time is %s\n",getpid(),date_buffer);
        fputs(pipe_output,fifoptr);
        sleep((unsigned int) (rand() % 6 + 1));
    }
    fclose(fifoptr);
    return 0;
}