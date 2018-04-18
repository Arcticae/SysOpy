#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define maxline_len 512
#define max_argnum 100
#define max_arglen 100

char**get_args(char*line){

    int i;
    char**result=malloc(max_argnum*sizeof(char*));
    for(i=0;i<max_argnum;i++){
        result[i]=malloc(max_arglen*sizeof(char));
    }
    i=0;
    while((result[i]=strtok(i==0?line:NULL,"\t \n"))!=NULL){
        i++;
    }
    return result;

}
int handle_oneliner(char*line){

    char*arguments[max_argnum];
    int executables=0,i=0;
    pid_t pid;

    while((arguments[executables]=strtok(executables == 0 ? line : NULL,"|"))!=NULL)executables++;
    int file_descriptors[executables][2];       //storing pipes

    for(i=0;i<executables;i++){



        if(pipe(file_descriptors[i])==-1) {
            perror("Error while doing the stuff with pipes! ");
            exit(EXIT_FAILURE);
        }

        pid=fork();
        if(pid==0){

            if(i!=executables-1){       //if it's not the last one you have to redirect the output

                if(dup2(file_descriptors[i][1],STDOUT_FILENO)==-1) {
                    fprintf(stderr,"Error while doing the stuff with dup2!(OUT)");
                    exit(EXIT_FAILURE);
                }
            }
            if(i>0){

                if(dup2(file_descriptors[i-1][0],STDIN_FILENO)==-1) {
                    fprintf(stderr,"Error while doing the stuff with dup2 (IN)%d!",i);
                    exit(EXIT_FAILURE);
                }
            }



            char**line_args=get_args(arguments[i]);
            execvp(line_args[0],line_args);
            perror("Piped task has encountered an error!");
            exit(EXIT_SUCCESS);

        }else{
            if(i<executables-1)close(file_descriptors[i][1]);  //closing the unused write end last needs to print to stdout the result
            if(i>0)close(file_descriptors[i-1][0]);    //previous in pipe needs to be closed
            wait(NULL);
        }

    }


    return 0;

}



int main(int argc, char**argv){

    if(argc<2){
        printf("You have given me not enough args, give me filepath\n");
        return -1;
    }

    FILE* batch_file=fopen(realpath(argv[1],NULL),"r");
    if(batch_file==NULL){
        printf("The batch file cannot be opened, please try again\n");
        return -1;
    }
    char line[maxline_len];

    while(fgets(line,maxline_len,batch_file)){
        fprintf(stderr,"\t%s\n",line);



            handle_oneliner(line);      //execution




    }


    fclose(batch_file);
    return 0;

}