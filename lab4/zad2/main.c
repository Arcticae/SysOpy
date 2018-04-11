//
// Created by timelock on 10.04.18.
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

volatile int N;      //children no
volatile int M;      //cap of sigusr1
volatile int received_returns=0;
volatile int received_sigs=0;
volatile int received_requests_number=0;

pid_t*children_pid;
pid_t*received_requests;





void set_action(int signum,struct sigaction*action){

    sigemptyset(&action->sa_mask);
    action->sa_flags=SA_SIGINFO;            //specified function must take 3 args, not one :" handler(int sig, siginfo_t *info, void *ucontext) "
    if(sigaction(signum,action,NULL)!=0){
        perror(errno);
    }
}


void sigint_handler(int signum,siginfo_t *info, void *ucontext){
    int i;
    printf("Received SIGINT, killing all children...\n");
    for(i=0;i<N;i++){
        if(children_pid!=0){
            printf("Killing child of pid:%d\n",children_pid[i]);
            kill(children_pid[i],SIGKILL);
            waitpid(children_pid[i],NULL,0);
        }
    }
    printf("Terminating...\n");
    exit(EXIT_SUCCESS);
}
void request_receive_handler(int signum,siginfo_t *info, void *ucontext){
    received_requests_number++;
    if(M>0){
        M--; //remaining processess reduced
        printf("SIGUSR1 received from the child: %d ,need %d more SIGUSR1s to start. \n",info->si_pid,M);
        received_requests[M]=info->si_pid;

        if(M==0){
            int i;
            printf("Reached the desired limit M, launching all the pending procesess...\n");

            for(i=0;i<received_requests_number;i++){
                printf("Pinging child %d with SIGUSR2\n",received_requests[i]);
                kill(received_requests[i],SIGUSR2);

            }
        }
    } else{
        printf("SIGUSR1 received from the child: %d ,pinging back with SIGUSR2\n",info->si_pid);
        kill(info->si_pid,SIGUSR2);
    }

}
void child_response_handler(int signum,siginfo_t *info, void *ucontext){
    received_sigs++;
    printf("Child : %d sent realtime sig: %d\n",info->si_pid,signum);
}
void child_terminated_handler(int signum,siginfo_t* info, void*ucontext){
    printf("Child : %d has terminated. It has slept for: %d seconds\n",info->si_pid,info->si_status);
    received_returns++;
    N--;
}

int main(int argc, char**argv){

    if(argc<3){
        printf("There are not enough arguments.\n");
        exit(EXIT_FAILURE);
    }
    N=(int)strtol(argv[1],NULL,10);
    M=(int)strtol(argv[2],NULL,10);

    children_pid=calloc(N,sizeof(pid_t));
    received_requests=malloc(N*sizeof(pid_t));
    int i;

    struct sigaction action;
    action.sa_handler=request_receive_handler;  //may cause errors?
    set_action(SIGUSR1,&action);
    action.sa_handler=sigint_handler;
    set_action(SIGINT,&action);
    action.sa_handler=child_terminated_handler;
    set_action(SIGCHLD,&action);
    action.sa_handler=child_response_handler;
    for(i=SIGRTMIN;i<=SIGRTMAX;i++){
        set_action(i,&action);
    }

    for(i=0;i<N;i++)    //creating N children
    {
        pid_t tmp_pid=fork();
        if(tmp_pid==0){
            execvp("./zad2_child",NULL);

            exit(EXIT_FAILURE);
        }else if(tmp_pid<0){
            printf("Failed fork\n");
            exit(EXIT_FAILURE);
        }
        children_pid[i]=tmp_pid;
    }



    while(N!=0){
        sleep(15);
        printf("Working processes: %d | Received requests for start: %d | Received realtime-sigs: %d\n ",N,received_requests_number,received_sigs);
    }
    printf("Returned total: %d\nReceived sigs: %d\n",received_returns,received_sigs);

    free(children_pid);
    free(received_requests);
    return 0;
}