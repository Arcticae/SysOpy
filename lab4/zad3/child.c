//
// Created by timelock on 11.04.18.
//
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


volatile int child_received_counter=0;
volatile int child_sent_counter=0;

//handlers
void handle_terminate(int signum){
    fprintf(stderr,"Child has terminated.\n");
    exit(EXIT_SUCCESS);
}


void handle_ping(int signum) {
    child_received_counter++;
    fprintf(stderr,"Child Received signal number: %d\n", child_received_counter);
    kill(getppid(),SIGUSR1);    //ping back
    child_sent_counter++;
}
//conf

void * get_sigaction(int sig){
    if(sig==SIGUSR2 || sig == SIGRTMAX){
        return handle_terminate;
    }else if(sig==SIGUSR1 || sig == SIGRTMIN){
        return handle_ping;
    }
}
void signal_handling(int sig1,int sig2){

    struct sigaction action;

    action.sa_sigaction=get_sigaction(sig1);
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask,sig1);

    if(sigaction(sig1,&action,NULL)!=0){
        fprintf(stderr,"Error has occured while setting signal response, reason: ");
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    action.sa_sigaction=get_sigaction(sig1);
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask,sig2);

    if(sigaction(sig2,&action,NULL)!=0){
        fprintf(stderr,"Error has occured while setting signal response, reason: ");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask,sig1);
    sigdelset(&action.sa_mask,sig2);
    sigprocmask(SIG_SETMASK, &action.sa_mask, NULL);

}
int main(int argc,char**argv){

    if(argc<2){
          fprintf(stderr,"Child process needs TYPE argument mr. programmer.\nTerminating child...\n");
          exit(EXIT_FAILURE);
      }

      int type=(int)strtol(argv[1],NULL,10);
      if( type == 1 || type == 2 ){
          signal_handling(SIGUSR1,SIGUSR2);
      }else if( type == 3 ){
          signal_handling(SIGRTMAX,SIGRTMIN);
      }else{
          fprintf(stderr,"Type arg is wrong, allowed 1-3 (child comm)\n");
          exit(EXIT_FAILURE);
      }

    while(1){sleep(1);fprintf(stderr,"im working\n");}

    exit(EXIT_SUCCESS);
}
