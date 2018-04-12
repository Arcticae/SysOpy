
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


volatile int sigs_sent = 0;
volatile int child_sigs_received = 0;
volatile int parent_sigs_received = 0;


pid_t child_pid;

void signal_handler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGINT) {
        printf("Parent: I have received interrupt!!!! I have to kill my child :( \n");
        kill(child_pid, SIGUSR2);
        exit(EXIT_FAILURE);
    }
    if ((signum == SIGUSR1 || signum == SIGRTMIN) && info->si_pid == child_pid) {
        parent_sigs_received++;
        printf("Parent: Received pingback from child Child\n");
    }
}

void sendsigs(int type,pid_t child_pid,int sigcount) {

    sleep(1);
    int i;

    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;

    if (sigaction(SIGINT, &action, NULL)!=0){
        printf("There was a problem whilst setting sig action\n");
    }

    if (type == 1 || type == 2){

        if (sigaction(SIGUSR1, &action, NULL)!=0){
            printf("There was a problem whilst setting sig action\n");
            exit(EXIT_FAILURE);
        }
    }
    if (type == 3) {
        if (sigaction(SIGRTMIN, &action, NULL)!=0){
            printf("There was a problem whilst setting sig action\n");
        }
    }

    if (type == 1 || type == 2) {

        sigset_t mask;          //used for ignoring other sigs in type 2
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGINT);

        for(i=0;i<sigcount;i++){
            printf("Parent: Pinging the child with SIGUSR1\n");
            kill(child_pid, SIGUSR1);
            if(type == 2)sigsuspend(&mask);       //here we wait for response if mode==2
            sigs_sent++;
        }
        printf("Parent: All signals sent. Pinging the child with SIGUSR2 (terminate)...\n");
        kill(child_pid, SIGUSR2);
    }
    if (type == 3) {
        for(i=0;i<sigcount;i++){
            printf("Parent: Pinging the child with realtime signals\n");
            kill(child_pid, SIGRTMIN);
            sigs_sent++;
        }
        sigs_sent++;
        printf("Parent: Sending Terminate signal\n");
        kill(child_pid, SIGRTMAX);
    }

    int status;
    waitpid(&status,&status,0);
    if (WIFEXITED(status)) child_sigs_received = WEXITSTATUS(status);
    else printf("Child had a problem whilst closing\n");
}


int main(int argc, char**argv) {

    if (argc < 3) {
        printf("You must give me 2 arguments - how many sigs you want to send and mode of sending\n");
        exit(EXIT_FAILURE);
    }

    int sigcount= (int)strtol(argv[1],NULL,10);
    int type= (int)strtol(argv[2],NULL,10);

    if(type < 1 || type > 3 || sigcount < 1) {
        printf("Please give me proper argument format for the love of god.\n");
        exit(EXIT_FAILURE);
    }

    printf("Parent: Sigcount: %d Type: %d\n",sigcount,type);
    if((child_pid= fork()) == 0){
        execl("./zad3_child","./zad3_child",argv[2]);
        exit(EXIT_FAILURE);
    }
    else if (child_pid>0) {
        sendsigs(type,child_pid,sigcount);
    }
    else fprintf(stderr,"Forking went wrong... .\n");

    printf("\nParent: signals sent: %d.\n", sigs_sent);
    printf("Parent: signals received from child: %d.\n", parent_sigs_received);

    return 0;
}
