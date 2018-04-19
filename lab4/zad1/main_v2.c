//
// Created by timelock on 10.04.18.
//
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

int sigtstp_wait=0;
int script_working=1;
pid_t child_pid=0;
void sigint_handler(int sigint){
    printf("\nOdebrano sygnał SIGINT\n");
    if(script_working){
        kill(child_pid,SIGKILL);
        script_working=0;
    }
    exit(EXIT_SUCCESS);
}
void sigtstp_handler(int sigtstp){
    if(sigtstp_wait==0)
    {
        printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        sigtstp_wait=1;
    }else sigtstp_wait=0;
    if(!script_working){
        //revive script
        script_working=1;
        if((child_pid=fork())==0){

            execvp(realpath("../zad1/script.sh",NULL),NULL);
            exit(EXIT_FAILURE);
        }

    }else{  //kill if working
        kill(child_pid,SIGKILL);
        script_working=0;
    }
}
void child_died(int sig){
    fprintf(stderr,"Execution of the child process was not possible, please check your args\n");
    exit(EXIT_FAILURE);
}
int main(int argc, char** argv) {//sigint = signal, sigtstp= sigaction
    struct sigaction sigact;
    if(argc<2){
        fprintf(stderr,"Give me filepath to script\n");
        exit(EXIT_FAILURE);
    }
    sigset_t sigset;
    sigemptyset(&sigset);

    signal(SIGCHLD,child_died);
    sigact.sa_handler=sigtstp_handler;
    sigact.sa_mask=sigset;
    sigact.sa_flags=0;

    signal(SIGINT,sigint_handler);
    sigaction(SIGTSTP,&sigact,NULL);

    if((child_pid=fork())==0){

        execvp(realpath(argv[1],NULL),NULL);
        perror("Following error occured executing the script");
        exit(EXIT_FAILURE);

    }
    if(child_pid)script_working=1;

    while(1){

        if(sigtstp_wait==0){ //when unstopped by sigtstp

        }
        sleep(1);

    }


    return 0;
}