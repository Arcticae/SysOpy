#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <string.h>
#include "lab7_sysv_common.h"
key_t sem_key;
int sems;
int shm_id;
shared_seg * memory;
void tear_down(int sig) {
    semctl(sems, 0, IPC_RMID);
    shmdt(memory);
    shmctl(shm_id,IPC_RMID,NULL);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    signal(SIGINT, tear_down);
    signal(SIGTSTP,tear_down);
    atexit(tear_down);
    //SEMAPHORES
    //SysV
    // 1) Get the key
    sem_key = ftok(getenv("HOME"), 's');
    // 2) Get semaphores set with semget
    if ((sems = semget(sem_key, 1,  IPC_CREAT |IPC_EXCL| S_IRWXU)) < 0)
        perror("SysV Sem creation error");
    // 3) Initial semaphore conditions
    semctl(sems, 0, SETVAL, 1);
    // SHARED MEMORY
    shm_id=shmget(sem_key,sizeof(shared_seg),IPC_CREAT | IPC_EXCL | S_IRWXU);
    memory=  (shared_seg*) shmat(shm_id,NULL,0);

    sleep(5);
    while (1) {
        struct sembuf sem_operation_get;
        sem_operation_get.sem_num = 0;
        sem_operation_get.sem_flg = 0;
        sem_operation_get.sem_op = -1;
        // 4) Get semaphore
        fflush(stdout);
        if (semop(sems, &sem_operation_get, (size_t) 1) < 0)
            perror("Sem get was not succ");
        printf("Parent got the semaphore! Previous value: %s\n", memory->buffer);
        strcpy(memory->buffer,"Parent was here\n");
        fflush(stdout);
        // 5) Give semaphore
        struct sembuf sem_operation_give;
        sem_operation_give.sem_num = 0;
        sem_operation_give.sem_flg = 0;
        sem_operation_give.sem_op = 1;
        if (semop(sems, &sem_operation_give, (size_t) 1) < 0)
            perror("Sem giv was not succ");
    }


    return 0;
}