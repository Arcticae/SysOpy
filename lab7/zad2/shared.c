//
// Created by timelock on 09.05.18.
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "shared.h"



void take_semaphore(sem_t * sem){
    if(sem_wait(sem)==-1){
        perror("Taking semaphore was unsuccesful");
    }
}

void give_semaphore(sem_t* sem){
    if(sem_post(sem)==-1){
        perror("Taking semaphore was unsuccesful");
    }
}

int queue_full(){
    if(barber->clients<barber->queue_size)return 0;
    else return 1;
}

int queue_empty(){
    if(barber->clients==0)return 1;
    else return 0;
}

void queue_push(pid_t pid){
    barber->fifo_queue[barber->clients++]=pid;
}

void queue_pop(){
    int i;
    for(i=0;i<barber->clients-1;i++)
        barber->fifo_queue[i]=barber->fifo_queue[i+1];
    barber->fifo_queue[--barber->clients]=0;
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
