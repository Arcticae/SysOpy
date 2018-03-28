//
// Created by timelock on 28.03.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define max_length 100
#define max_argnum 20

int main(int argc, char**argv){

    if(argc<2){
        printf("You have given me not enough args, give me filepath\n");
        return -1;
    }

    FILE* batch_file=fopen(argv[1],"r");
    if(batch_file==NULL){
        printf("The batch file cannot be opened, please try again\n");
        return -1;
    }
    char*arguments[max_argnum];
    char line[max_length];
    int argnum,i,status;

    while(fgets(line,max_length,batch_file)){
        argnum=0;
        do{
            if(argnum==0){
                arguments[argnum]=strtok(line,"\t\n ");
                argnum++;
            }else {
                arguments[argnum]=strtok(NULL,"\t\n ");
                argnum++;
            }

        }while(arguments[argnum-1]!=NULL && argnum < max_argnum);

        if(argnum>=max_argnum){
            printf("The number of argumnets has exceeded the given limit. Please reedit the given file\n");
            return -1;
        }

        pid_t child_pid=fork();
        if(child_pid==0){
            execvp(arguments[0],arguments);
            exit(1);                       //the code below will not be executed if the process does exec properly
        }else if(child_pid<0){
            printf("Fork failed.\n");
            return -1;
        }
        wait(&status);

        if(status!=0){
            printf("The process: %s returned code %d, the task will now terminate.\n",arguments[0],status);
            return -1;
        }
    }



    fclose(batch_file);
    return 0;
}