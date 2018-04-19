#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

    int child_received=0;
    int parent_sent=0;
    int child_resent=0;
    int child_terminated=0;

    pid_t child_pid;
    pid_t parent_pid;

void parent_s1_handle(int sig){
    printf("Parent: Received signal no. %d  number %d\n",sig,++child_resent);
}

void parent_s2_handle(int sig, siginfo_t *info, void *ucontext) {
    child_received = info->si_value.sival_int;
    child_terminated = 1;
}

void child_s1_handle(int sig){
    printf("Child: Received signal no. %d  number %d\n",sig,++child_received);
    kill(parent_pid, sig);
}

void child_s2_handle(int sig){
    printf("Received sig no :%d in child process\nTerminating child...\n",sig);
    union sigval received_value;
    received_value.sival_int = child_received;
    sigqueue(parent_pid, sig, received_value);
    exit(EXIT_SUCCESS);
}

void child_prog(int sig1, int sig2){

    parent_pid= getppid();

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, sig1);
    sigdelset(&set, sig2);
    sigprocmask(SIG_SETMASK, &set, NULL);

    struct sigaction child_act_s1;
    child_act_s1.sa_handler = child_s1_handle;
    sigemptyset(&child_act_s1.sa_mask);
    sigaddset(&child_act_s1.sa_mask, sig2);
    child_act_s1.sa_flags = 0;
    sigaction(sig1, &child_act_s1, NULL);

    struct sigaction child_act_s2;
    child_act_s2.sa_handler = child_s2_handle;
    sigemptyset(&child_act_s2.sa_mask);
    child_act_s2.sa_flags = 0;
    sigaction(sig2, &child_act_s2, NULL);

    while(1){sleep(1);};
}

int main(int argc, char **argv){
    if (argc < 3) {
        printf("You must give me 2 arguments - how many sigs you want to send and mode of sending\n");
        exit(EXIT_FAILURE);
    }

    int sigcount= (int)strtol(argv[1],NULL,10);
    int type= (int)strtol(argv[2],NULL,10);
    int sig1,sig2;

    if(type < 1 || type > 3 || sigcount < 1) {
        printf("Please give me proper argument format for the love of god.\n");
        exit(EXIT_FAILURE);
    }
    if(type==3){
        sig1=SIGRTMIN;
        sig2=SIGRTMAX;
    }else{
        sig1=SIGUSR1;
        sig2=SIGUSR2;
    }

    struct sigaction sigaction1;
    sigaction1.sa_handler = parent_s1_handle;
    sigemptyset(&sigaction1.sa_mask);
    sigaction1.sa_flags = 0;
    sigaction(sig1, &sigaction1, NULL);


    struct sigaction sigaction2;
    sigaction2.sa_sigaction = parent_s2_handle;
    sigemptyset(&sigaction2.sa_mask);
    sigaction2.sa_flags = SA_SIGINFO;
    sigaction(sig2, &sigaction2, NULL);

    child_pid = fork();
    if(child_pid == 0)child_prog(sig1, sig2);//execute
    sleep(3);
    for(int i = 0; i < sigcount; i++) {
        if(kill(child_pid, sig1)==0)printf("Parent: Sent signal of number %d and count %d to child\n",sig1,++parent_sent);
        else fprintf(stderr,"Parent: Could not send signal of number %d %d\n",sig1, ++parent_sent);
        if(type == 2) while(parent_sent !=child_resent) {};        //wait for sent to be equal to received
    }

    if(kill(child_pid, sig2)==0)printf("Sent signal TERMINATE of number %d to child \n",sig2);
    else fprintf(stderr,"Could not send signal TERMINATE of number %d to child \n",sig2);

    while(child_terminated!=1) {sleep(1);}
    sleep(1);
    fprintf(stderr,"Child has received %d total sigs\nParent has sent %d total sigs\n Child has resent %d total sigs\n",child_received,parent_sent,child_resent);
        //check for sigprocmask, and masks in general. Above may be printed off in sigchld
}