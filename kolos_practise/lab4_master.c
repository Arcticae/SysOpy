
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void handle_sigtstp(int sig){
    printf("Got signal SIGTSTP nr:%d\n",sig);
}
void handle_sigalarm(int sig){
    printf("Got signal SIGALARM nr:%d\n",sig);
}

int main(int argc, char **argv) {

    printf("My pid is :%d", getpid());
    //kill(getpid(),SIGTSTP);

    signal(SIGTSTP,handle_sigtstp);
    signal(SIGALRM,handle_sigalarm);
  /*  while(1){
        printf("\nWorking...");
        sleep(5);
        raise(SIGALRM);     //equivalent of kill(getpid(),SIGALRM)
    }
    */
    sigval_t values;
    values.sival_int=10;
    sigval_t values2;
    values2.sival_int=20;

    while(1){
        sigqueue((int)strtol(argv[1],NULL,10),SIGRTMIN,values);
        sleep(5);
        sigqueue((int)strtol(argv[1],NULL,10),SIGRTMAX,values2);
        sleep(5);
    }


    return 0;
}