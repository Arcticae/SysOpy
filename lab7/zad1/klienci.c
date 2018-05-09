//
// Created by timelock on 09.05.18.
//
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "shared.h"

my_queue *clients = NULL;
key_t queue_key;

int semgroup_id;
int shared_memory_id;

volatile int cuts_handled = 0;
int cuts_required;
int clients_number;

void exit_procedure() {
    if (shmdt(clients) == -1) {
        perror("Detaching mem not succesful");
        exit(EXIT_FAILURE);
    }
}

void setup_queue() {

    shared_memory_id = shmget(queue_key, 0, 0);
    if (shared_memory_id == -1) {
        perror("Shared memory open failed");
        exit(EXIT_FAILURE);
    }

    clients = (my_queue *) shmat(shared_memory_id, NULL, 0);

    if (clients == (my_queue *) -1) {
        perror("Attaching shared mem to address space of process has failed, reason");
        exit(EXIT_FAILURE);
    }

}

void setup_sems() {

    semgroup_id = semget(queue_key, 0, 0);

    if (semgroup_id == -1) {
        perror("Getting semaphores failed, reason");
        exit(EXIT_FAILURE);
    }

}
volatile int received=0;
void cut_received(int signum) {
    cuts_handled++;
    received=1;
}

int get_cut(){

    int barber_asleep=semctl(semgroup_id,AWAKE,GETVAL);
    if(barber_asleep==-1){
        perror("Barber semaphore lookup not possible");
        exit(EXIT_FAILURE);
    }
    if(barber_asleep==0){   //means he is asleep ;-D
        struct sembuf action;
        action.sem_op=1;        //dajemy semaforke
        action.sem_flg=0;
        action.sem_num=AWAKE;

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Giving AWAKE semaphore not succesful");
            exit(EXIT_FAILURE);
        }

        printf("Timestamp: %ld | Client %d has awakened the golibroda! \n",get_time(),getpid());
        fflush(stdout);

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Giving AWAKE semaphore not succesful");
            exit(EXIT_FAILURE);
        }                               //one of the sems is just awake, second one to give him the zlecenie

        clients->chair=getpid();
        printf("Timestamp: %ld | Client %d has taken the seat.\n",get_time(),getpid());
        return 1;
    } else{

            if(queue_push(clients,getpid())!=-1){
            printf("Timestamp: %ld | Client %d took a place in queue \n",get_time(),getpid());
            fflush(stdout);
            return 1;
            }else{
                printf("Timestamp: %ld | Client %d could not get place in queue as its full \n",get_time(),getpid());
                fflush(stdout);
                return -1;
        }

    }

}
void launch_child(){

    /*
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGRTMIN);
    */
    int i;
    /*while(cuts_handled<cuts_required)*/for(i=0;i<cuts_required;i++){
        struct sembuf action;
        action.sem_flg=0;
        action.sem_num=CHECK;
        action.sem_op=-1;

        if (semop(semgroup_id,&action, 1) == -1) {
            perror("Taking CHECK semaphore not succesful");       //TODO
            exit(EXIT_FAILURE);
        }

        action.sem_num=QUEUE;

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Taking QUEUE semaphore not succesful");
            exit(EXIT_FAILURE);
        }

        int done=get_cut();
      //  received=0;

        action.sem_op=1;

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Giving QUEUE semaphore not succesful");
            exit(EXIT_FAILURE);
        }

        action.sem_num=CHECK;

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Giving QUEUE semaphore not succesful");
            exit(EXIT_FAILURE);
        }

        //sigwait(&sigset,NULL);
        //fprintf(stderr,"AFTER WAIT\n");

        if(done==1){
            printf("Timestamp: %ld | Client %d has received a cut from barber\n",get_time(),getpid());
            fflush(stdout);
        }
        //while(received!=1)wait(NULL);

    }

}


int main(int argc, char **argv) {

    if (argc < 3) {
        printf("Please give me clients no and cuts for each\n");
        exit(EXIT_FAILURE);
    }
    clients_number = (int) strtol(argv[2], NULL, 10);
    cuts_required = (int) strtol(argv[3], NULL, 10);

    if (cuts_required < 1 ) {
        printf("Please give correct no of cuts req\n");
        exit(EXIT_FAILURE);
    }

    if (clients_number < 1 || clients_number > MAXQUEUE_SIZE) {
        printf("Please give correct no clients btwn 0 and max\n");
        exit(EXIT_FAILURE);
    }

    queue_key = QUEUE_KEY;

    atexit(exit_procedure);

    setup_sems();
    setup_queue();
   // signal(SIGRTMIN,cut_received);

    int i;
    for(i=0;i<clients_number;i++){

        pid_t chld_pid=fork();
        if(chld_pid==0){
            launch_child();
            return 0;
        }
    }
    while(1){
        wait(NULL);
        if(errno==ECHILD){break;}
    }
    printf("Timestamp: %ld | Client manager : All children have been cut.\n",get_time());
    return 0;
}