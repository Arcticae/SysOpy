//
// Created by timelock on 09.05.18.
//
#define _GNU_SOURCE


#include "shared.h"

my_queue *clients = NULL;
unsigned max_clients;

int semgroup_id;
int shared_memory_id;

const char*queue_sem_path="/queue";
const char*check_sem_path="/check";
const char*asleep_sem_path="/asleep";
const char*clients_mem_path="/clients";
sem_t *QUEUE;
sem_t *ASLEEP;
sem_t *CHECK;


volatile int cuts_handled = 0;
int cuts_required;
int clients_number;

void exit_procedure() {
    if (munmap(clients, sizeof(my_queue)) == -1) {
        perror("Detaching mem not succesful");
        exit(EXIT_FAILURE);
    }

    if (sem_close(QUEUE) == -1 ||
        sem_close(ASLEEP) == -1 ||
        sem_close(CHECK) == -1) {
        perror("Closing sems not succesful");
        exit(EXIT_FAILURE);
    }

}

void setup_queue() {
    shared_memory_id = shm_open(clients_mem_path, O_RDWR, 0666);
    if ((clients = (my_queue *) mmap(NULL, sizeof(my_queue), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0)) == (my_queue *) -1) {
        perror("Cannot map");
        exit(EXIT_FAILURE);
    }
}

void setup_sems() {

    if ((ASLEEP = sem_open(asleep_sem_path, O_RDWR)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }
    if ((QUEUE = sem_open(queue_sem_path, O_RDWR)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }
    if ((CHECK = sem_open(check_sem_path, O_RDWR)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }

}

volatile int received = 0;

void cut_received(int signum) {
    cuts_handled++;
    received = 1;
}

int get_cut() {
    int barber_asleep;
    sem_getvalue(ASLEEP, &barber_asleep);
    if (barber_asleep == -1) {
        perror("Barber semaphore lookup not possible");
        exit(EXIT_FAILURE);
    }
    if (barber_asleep == 0) {   //means he is asleep ;-D
        sem_post(ASLEEP);       //wake him up
        printf("Timestamp: %ld | Client %d has awakened the golibroda! \n", get_time(), getpid());
        fflush(stdout);
        clients->chair = getpid();

        printf("Timestamp: %ld | Client %d has taken the seat.\n", get_time(), getpid());
        return 1;
    } else {

        if (queue_push(clients, getpid()) != -1) {
            printf("Timestamp: %ld | Client %d took a place in queue \n", get_time(), getpid());
            fflush(stdout);
            return 1;
        } else {
            printf("Timestamp: %ld | Client %d could not get place in queue as its full \n", get_time(), getpid());
            fflush(stdout);
            return -1;
        }

    }

}

void launch_child() {

    /*
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGRTMIN);
    */
    int i;
    /*while(cuts_handled<cuts_required)*/for (i = 0; i < cuts_required; i++) {
        if (sem_wait(CHECK) == -1) {
            perror("Wait 4 check failed");
            exit(EXIT_FAILURE);
        }

        if (sem_wait(QUEUE) == -1) {
            perror("Wait 4 queue failed");
            exit(EXIT_FAILURE);
        }


        int done = get_cut();
        //  received=0;

        if (sem_post(QUEUE) == -1) {
            perror("Posting queue failed");
            exit(EXIT_FAILURE);
        }

        if (sem_post(CHECK) == -1) {
            perror("Posting check failed");
            exit(EXIT_FAILURE);
        }


        //sigwait(&sigset,NULL);
        //fprintf(stderr,"AFTER WAIT\n");

        if (done == 1) {
            printf("Timestamp: %ld | Client %d is leaving the barbershop\n", get_time(), getpid());
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

    if (cuts_required < 1) {
        printf("Please give correct no of cuts req\n");
        exit(EXIT_FAILURE);
    }

    if (clients_number < 1 || clients_number > MAXQUEUE_SIZE) {
        printf("Please give correct no clients btwn 0 and max\n");
        exit(EXIT_FAILURE);
    }


    atexit(exit_procedure);

    setup_sems();
    setup_queue();
    // signal(SIGRTMIN,cut_received);

    int i;
    for (i = 0; i < clients_number; i++) {

        pid_t chld_pid = fork();
        if (chld_pid == 0) {
            launch_child();
            return 0;
        }
    }
    while (1) {
        wait(NULL);
        if (errno == ECHILD) { break; }
    }
    printf("Timestamp: %ld | Client manager : All children have been cut.\n", get_time());
    return 0;
}