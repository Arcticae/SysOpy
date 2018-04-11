//
// Created by timelock on 11.04.18.
//

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int consent=0;

void sigusr2_handler(int signum){

    if(signum==SIGUSR2)consent=1;
}

int main(){
    srand(getpid());
    int time=(unsigned int)(rand()%11);
    sleep(time);
    kill(getppid(),SIGUSR1);    //send_request for continuing work
    signal(SIGUSR2,sigusr2_handler);

    while(consent!=1){
        sleep(1);
    }
    kill(getppid(),rand()%(SIGRTMAX-SIGRTMIN+1) +SIGRTMIN);

    return time;
}