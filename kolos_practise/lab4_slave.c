

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <bits/siginfo.h>
#include <bits/sigaction.h>

void sigrtmin_handler(int sig, siginfo_t *info, void *ignore) {
    printf("Got SIGRTMIN, value: %d\n",info->si_value.sival_int);
}

void sigrtmax_handler(int sig,  siginfo_t *info, void *ignore) {
    printf("Got SIGRTMAX, value: %d\n",info->si_value.sival_int);
}


int main(int argc, char **argv) {

    //signal(SIGRTMIN, sigrtmin_handler);
    //signal(SIGRTMAX, sigrtmax_handler);
    //structure for sigaction
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    //this sets action for flag=SA_SIGINFO
    action.sa_sigaction = &sigrtmin_handler;
    sigaction(SIGRTMIN, &action, NULL);
    //action.sa_mask is of type sigset_t and needs to
    action.sa_sigaction = &sigrtmax_handler;
    sigemptyset(&action.sa_mask);
    sigset_t set_blocked_by_proccess;
    sigemptyset(&set_blocked_by_proccess);
    sigaddset(&set_blocked_by_proccess,SIGRTMIN);
    sigaddset(&action.sa_mask,SIGRTMIN);
    sigaction(SIGRTMAX, &action, NULL);
    printf("My pid is %d\n",getpid());
    sigprocmask(SIG_BLOCK,&set_blocked_by_proccess,NULL);

    while (1) {
        printf("Slave: Doing work...\n");
        sleep(3);
    }


    return 0;
}
