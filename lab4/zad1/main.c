//
// Created by timelock on 10.04.18.
//
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
int sigtstp_wait=0;
void sigint_handler(int sigint){
    printf("Odebrano sygnał SIGINT\n");
    exit(EXIT_SUCCESS);
}
void sigtstp_handler(int sigtstp){
    if(sigtstp_wait==0) {
    printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
    sigtstp_wait=1;

    }else sigtstp_wait=0;

}
int main(int argc, char** argv) {//sigint = signal, sigtstp= sigaction
    struct sigaction sigact;

    sigset_t sigset;
    sigemptyset(&sigset);


    sigact.sa_handler=sigtstp_handler;
    sigact.sa_mask=sigset;
    sigact.sa_flags=0;

    signal(SIGINT,sigint_handler);
    sigaction(SIGTSTP,&sigact,NULL);

    while(1){

        if(sigtstp_wait==0){
        time_t currtime=time(NULL);
        char*curtime_str=ctime(&currtime);
        printf("Time is: %s\n",curtime_str);
        }
        sleep(1);

    }


    return 0;
}