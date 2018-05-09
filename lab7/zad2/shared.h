//
// Created by timelock on 09.05.18.
//

#ifndef LAB7_SHARED_H
#define LAB7_SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXQUEUE_SIZE 2048



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
