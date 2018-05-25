//
// Created by timelock on 09.05.18.
//

#ifndef LAB7_SHARED_H
#define LAB7_SHARED_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "shared.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define MAXQUEUE_SIZE 1024
#define QUEUE_KEY 45678

enum barber_status {
    SLEEPING,
    AWAKEN,
    READY,
    IDLE,
    SHAVING
};

enum client_status {
    ARRIVED,
    INVITED,
    SHAVED
};

struct barber_info {
    enum barber_status barber_status;
    int clients;
    int queue_size;
    pid_t chair;
    pid_t fifo_queue[MAXQUEUE_SIZE];
} *barber;


__syscall_slong_t get_time();
void take_semaphore(int sem_id);
void give_semaphore(int sem_id);
int queue_full();
int queue_empty();
void queue_push(int pid);
void queue_pop();


#endif //LAB7_SHARED_H
