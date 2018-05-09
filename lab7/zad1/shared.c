//
// Created by timelock on 09.05.18.
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "shared.h"

void initialize_queue(my_queue *queue, unsigned size) {
    queue->max_size = size;
    queue->head = -1;
    queue->tail = 0;
    queue->chair = 0;
}


int is_empty(my_queue *queue) {
    if (queue->head == -1) return 1;
    else return 0;
}

int is_full(my_queue *queue) {

    if (queue->head == queue->tail) return 1;
    else return 0;

}


pid_t queue_pop(my_queue *queue) {

    if (is_empty(queue))return -1;                           //error

    queue->chair = queue->queue[queue->head++];              //move the head pointer one step closer to the tail

    if (queue->head == queue->max_size)queue->head = 0;      //reached end of array, reset to the beginning

    if (queue->head == queue->tail)queue->head = -1;         //this means queue is full

    return queue->chair;
}

int queue_push(my_queue *queue, pid_t client) {
    if (is_full(queue))return -1;
    if (is_empty(queue)) {
        queue->tail = queue->head = 0;      //reset the queue to 0
    }
    queue->queue[queue->tail++] = client;

    if (queue->tail == queue->max_size)queue->tail = 0;

    return 0;

}

__syscall_slong_t get_time() {

    struct timespec timestamp;
    if (clock_gettime(CLOCK_MONOTONIC, &timestamp)) {
        perror("Couldn't get timestamp, reason:");
        exit(EXIT_FAILURE);
    }

    return timestamp.tv_nsec / 1000;

    //return value needs to be cast
}
