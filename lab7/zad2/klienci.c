//
// Created by timelock on 09.05.18.
//
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include "shared.h"

enum client_status my_state;

sem_t * sem;
int shared_memory_id;
int cuts_required;

void queue_contains(){
    printf("Queue contains pids: (%d of them)\n",barber->clients);
    int i;
    for(i=0;i<barber->queue_size;i++)printf("Place %d, pid :%d\n",i,barber->fifo_queue[i]);
    fflush(stdout);
}

void setup_client_manager() {       //DONE

    shared_memory_id=shm_open(keypath,O_RDWR,S_IRWXU|S_IRWXG);
    if(shared_memory_id == -1) {
        perror("Shared memory creation was unsuccsesful");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shared_memory_id, sizeof(*barber))==-1){
        perror("Trunc ntot succc");
        exit(EXIT_FAILURE);
    }
    if ((barber = (struct barber_info*) mmap(NULL, sizeof(struct barber_info), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0)) == (struct barber_info*) -1) {
        perror("Cannot map barber info");
        exit(EXIT_FAILURE);
    }

    if((sem=sem_open(keypath,O_WRONLY,S_IRWXU|S_IRWXG,0))==(void*)-1){
        perror("Cannot open semaphore");
        exit(EXIT_FAILURE);
    }

}

void get_seated(){
    pid_t my_pid=getpid();

    if(my_state==INVITED){
        queue_pop();
    }else if (my_state==ARRIVED){

        while(1){
            give_semaphore(sem);
            take_semaphore(sem);
            if(barber->barber_status==READY)break;

        }

        my_state=INVITED;
    }

    barber->chair=my_pid;        //taking seat
    printf("Timestamp: %ld | Client no: %d has taken the seat ibiza\n", get_time(), my_pid);
    fflush(stdout);
}

void launch_child() {        //DONE
    pid_t my_pid = getpid();
    int cuts = 0;

    while (cuts < cuts_required) {
        my_state = ARRIVED;

        take_semaphore(sem);

        if (barber->barber_status == SLEEPING) {

            printf("Timestamp: %ld | Client no: %d has awaken the golibroda\n", get_time(), my_pid);
            fflush(stdout);

            barber->barber_status = AWAKEN;
            get_seated();
            barber->barber_status = SHAVING;
        } else if (!queue_full()) {
            queue_push(my_pid);
            printf("Timestamp: %ld | Client no: %d has entered the queue\n", get_time(), my_pid);
            //queue_contains();
            fflush(stdout);

        } else {
            printf("Timestamp: %ld | Client no: %d could not enter the queue as it's full\n",
                   get_time(), my_pid);
          
           // queue_contains();
            fflush(stdout);
            give_semaphore(sem);
            continue;
        }

        give_semaphore(sem);


        while (my_state != INVITED) {
            take_semaphore(sem);
            if (barber->chair == my_pid) {
                my_state = INVITED;
                get_seated();
                barber->barber_status = SHAVING;
            }
            give_semaphore(sem);
        }

        while (my_state != SHAVED) {
            take_semaphore(sem);
            if (barber->chair != my_pid) {
                my_state = SHAVED;
                printf("Timestamp: %ld | Client no: %d has been shaved\n", get_time(), my_pid);
                fflush(stdout);
                barber->barber_status = IDLE;
                cuts++;
            }
            give_semaphore(sem);
        }

    }
    printf("Timestamp: %ld | Client no: %d has received all the cuts\n", get_time(), my_pid);
    fflush(stdout);
}


int main(int argc, char **argv) {

    if (argc < 3) {
        printf("Please give me clients no and cuts for each\n");
        exit(EXIT_FAILURE);
    }
    int clients_number = (int) strtol(argv[1], NULL, 10);
    cuts_required = (int) strtol(argv[2], NULL, 10);

    if (cuts_required < 1) {
        printf("Please give correct no of cuts req\n");
        exit(EXIT_FAILURE);
    }

    if (clients_number < 1 || clients_number > MAXQUEUE_SIZE) {
        printf("Please give correct no clients btwn 0 and max\n");
        exit(EXIT_FAILURE);
    }


    setup_client_manager();

    int i;
    for (i = 0; i < clients_number; i++) {

        if (fork() == 0) {
            launch_child();
            exit(EXIT_SUCCESS);
        }
    }
    while (1) {
        wait(NULL);
        if (errno == ECHILD) { break; }       //means all children have been terminated
    }
    printf("Timestamp: %ld | Client manager : All children have launched.\n", get_time());
    return 0;
}