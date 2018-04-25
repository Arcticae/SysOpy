//
// Created by timelock on 25.04.18.
//

#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "queuedetails.h"

int client_qid;
int server_qid;

key_t client_id;
key_t server_id;

int id_index;
int sent=0;
int mother=0;
void handle_interrupt(int sig){
    printf("I have received signal SIGINT, terminating...\n");
    exit(0);
}

void exit_procedure(){


    msgctl(client_qid,IPC_RMID,NULL);        //remove queue
    if(mother==1){      //we need only to say goodbye when we know mother is alive.
        struct msgbuffer_new exit_message;
        exit_message.mtype=GOODBYE;
        exit_message.index=id_index;
        sprintf(exit_message.message,"CLNT_TERM");
        if(msgsnd(server_qid,&exit_message,MESSAGE_SIZE,0)!=0){     //say gbye to srv
            perror("Could not say goodbye to srv, reason");
            exit(EXIT_FAILURE);
        }
    }


}

void start_client(){

    atexit(exit_procedure);
    signal(SIGINT,handle_interrupt);
    char*home=getenv("HOME");


    server_id=ftok(home,'s');            //s stands for serv

    if(server_id==-1){
        perror("Error while getting the queuekey for srv");
        exit(EXIT_FAILURE);
    }

    client_id=ftok(home,getpid());

    if(client_id==-1){
        perror("Error while getting the queuekey for clnt");
        exit(EXIT_FAILURE);
    }

    server_qid=msgget(server_id, S_IRWXU | S_IRWXG );

    if(server_qid==-1){
        perror("Error while creating the queue for srv");
        exit(EXIT_FAILURE);
    }

    client_qid=msgget(client_id,IPC_CREAT | IPC_EXCL |S_IRWXU | S_IRWXG);

    if(client_qid==-1){
        perror("Error while creating the queue for clnt");
        exit(EXIT_FAILURE);
    }

}

void client_introduce(){
    struct msgbuffer_new message;
    struct msgbuffer_new response;

    message.mtype=INTRODUCE;
    message.queue_key=client_id;
    strcpy(message.message,"INTRODUCE_CLNT");
    if(msgsnd(server_qid,&message,MESSAGE_SIZE,0)!=0){
        perror("Sending message INTRODUCE was not succesful, reason");
        exit(EXIT_FAILURE);
    }

    if(msgrcv(client_qid,&response,MESSAGE_SIZE,INTRODUCE,0)==-1){
        perror("Receiving response to INTRODUCE was not succesful, reason");
        exit(EXIT_FAILURE);
    }

    id_index=response.index;            //assignment of given index by the server, to id next msgs

}

int main(int argc,char**argv){

    if(argc<1){
        fprintf(stderr,"please give me filepath to interpret commands from\n");
        exit(EXIT_FAILURE);
    }

    FILE*batchfile;

    char*valid_strings[]={"MIRROR","ADD","SUB","MUL","DIV","TIME","END"}; //needed 4 stuff

    start_client();
    client_introduce();


    if((batchfile=fopen(argv[1],"r"))==NULL){
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }


        char buffer[MESSAGE_SIZE];
        char*temp_buffer;
        int i;
        mother=1;

        while(fgets(buffer,MESSAGE_SIZE,batchfile)){

              temp_buffer=strtok(buffer," \n\t");  //we have in the buffer the name of command
              int valid=0;
              struct msgbuffer_new message;

              for(i=0;i < sizeof(valid_strings)/sizeof(char*) && temp_buffer!=NULL ;i++){
                  if(strcmp(valid_strings[i],temp_buffer)==0){

                      valid=1;

                      message.mtype=1+i;                    //because of enumeration from 1
                      strcpy(message.message,temp_buffer); //copy the command.
                      message.index=id_index;              //for server response.

                      temp_buffer=strtok(NULL,"\n");
                      if(temp_buffer!=NULL){
                          strcpy(message.message,temp_buffer); //validation of args in server!
                      }
                      if(temp_buffer==NULL && message.mtype!=TIME && message.mtype!= END){
                          valid=0;      //two step validation
                      }
                      if(temp_buffer!=NULL && (message.mtype==TIME || message.mtype==END)){
                          valid=0;
                      }
                  }
              }

            if(valid==0){
                  fprintf(stderr,"Line %s contains invalid command!\n",buffer);
                  continue;
            }else if(msgsnd(server_qid,&message,MESSAGE_SIZE,0)==1){
                perror("Error sending comms off to srv");
                exit(EXIT_FAILURE);
            }
            printf("Sent msg %s of opcode %li\n",message.message,message.mtype);
            sent++;
        }

    while(sent!=0){//send off all then wait for responses.
        struct msgbuffer_new response;

        if(msgrcv(client_qid,&response,MESSAGE_SIZE,0,0)==-1){
            perror("Error while getting the response from srv");
            exit(EXIT_FAILURE);
        } else{
            if(response.mtype==END){
                printf("Mother program has come to an end, terminating now.\n");
                mother=0;
                exit(EXIT_SUCCESS);
            }
            printf("Got response for an op: %s, result: %s\n",valid_strings[response.mtype-1],response.message);
            sent--;
        }
    }

    return 0;
}
