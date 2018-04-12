//
// Created by timelock on 11.04.18.
//
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

volatile int signals_returned=0;
volatile int signals_sent=0;

void handle_returned_sig(int signum){
    signals_returned++;
    printf("Parent Received signal number: %d which is sig: %d\n", signals_returned,signum);
}
void chld_die(int signum,siginfo_t *info, void *ucontext){
    printf("Child process has terminated, status: %d\n",info->si_status);
    exit(EXIT_FAILURE);
}



int main(int argc,char**argv){

    if(argc<3){
        printf("You must tell me how many signals to send, and what is the TYPE variable (1  - serialize, 2 - one by one, 3 - realtime sigs \nTerminating...\n");
        exit(EXIT_FAILURE);
    }
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_handler=chld_die;

    sigaction(SIGCHLD,&action,NULL);

    int how_many=(int)strtol(argv[1],NULL,10);
    int type=(int)strtol(argv[2],NULL,10);


    pid_t child_pid;
    if((child_pid=fork())==0){
        execl("./zad3_child","./zad3_child",argv[2]);
        printf("Oops, something went wrong while executing child.\n");
        exit(EXIT_FAILURE);
    }

    int i;
    if(type==1 || type == 2){
        signal(SIGUSR1,handle_returned_sig);
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask,SIGUSR1);


        for(i=0;i<how_many;i++){
            kill(child_pid,SIGUSR1);
            if(type==2)sigsuspend(&mask);
            signals_sent++;
        }

        kill(child_pid,SIGUSR2);
    }else if (type==3){

        for(i=0;i<how_many;i++){
            kill(child_pid,SIGRTMIN);

            signals_sent++;
        }
        kill(child_pid,SIGRTMAX);
    }
    int status;
    waitpid(child_pid,&status,0);
    printf("Status:%d\n",status);

    printf("Signals returned: %d\nSignals sent: %d\n",signals_returned,signals_sent);


    return 0;
}