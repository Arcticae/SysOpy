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

#define MAXQUEUE_SIZE 2048
#define QUEUE_KEY 45678

//define some semaphore ids

#define AWAKE 0
#define QUEUE 1
#define CHECK 2
#define SOMETHING 3

typedef struct my_queue{
    int max_size;
    int head;
    int tail;
    pid_t queue[MAXQUEUE_SIZE];
    pid_t chair;

}my_queue;

__syscall_slong_t get_time();
void initialize_queue(my_queue *queue, unsigned size);
int is_empty(my_queue *queue);
int is_full(my_queue *queue);
pid_t queue_pop(my_queue *queue);
int queue_push(my_queue *queue, pid_t client);



#endif //LAB7_SHARED_H
