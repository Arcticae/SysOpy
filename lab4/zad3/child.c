#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int pings_received = 0;


void sighandler(int signal,siginfo_t *info, void *ucontext){
    if(signal==SIGUSR1 || signal==SIGRTMIN){
        pings_received++;
        fprintf(stderr,"Child: Received ping, pingback on its way\n");
        kill(getppid(),SIGUSR1);
    }else if(signal==SIGUSR2 || signal==SIGRTMAX){
        fprintf(stderr,"Child: Received sig to terminate\nChild: Total received sigs: %d\n",pings_received);
        exit(EXIT_SUCCESS);
    }
}

void handle_signals(int sig1, int sig2){

        struct sigaction action;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = sighandler;

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, sig1);
        sigdelset(&mask, sig2);

        if (sigaction(sig1, &action, NULL)!=0){
            fprintf(stderr,"Child: There was a problem whilst setting sighandle.\n");
        }
        if (sigaction(sig2, &action, NULL)!=0){
            fprintf(stderr,"Child: There was a problem whilst setting sighandle.\n");
        }

        sigprocmask(SIG_SETMASK, &mask, NULL);
}

int main(int argc, char **argv){

    int type= (int)strtol(argv[1],NULL,10);
    if(type==1 || type == 2 )handle_signals(SIGUSR1,SIGUSR2);
    else if(type==3)handle_signals(SIGRTMIN,SIGRTMAX);

    while(1){
        sleep(1);
    }
}