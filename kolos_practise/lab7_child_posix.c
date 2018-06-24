#include <sys/types.h>
#include <stdlib.h>
#include "lab7_posix_common.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

sem_t *semaphore;
shared_seg *memory;

void tear_down(int sig) {
    sem_close(semaphore);
    munmap(memory,sizeof(shared_seg));
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    signal(SIGINT, tear_down);
    signal(SIGTSTP, tear_down);
    atexit(tear_down);
    //SEMAPHORES
    //POSIX

    int shm_key=shm_open("/shm_posix",O_RDWR,0);
    ftruncate(shm_key,sizeof(shared_seg));
    memory=mmap(NULL,sizeof(shared_seg),PROT_READ | PROT_WRITE , MAP_SHARED,shm_key,0);


    // 1) Get the sem
    semaphore = sem_open("/semaforka", O_RDWR , 0, 0);
    // 2) Get semaphores set with semget

    sleep(5);
    while (1) {
        // 4) Get semaphore
        if (sem_wait(semaphore)<0)
            perror("Sem get was not succ");
        printf("Child got the semaphore! Previous content: %s\n", memory->buffer);
        fflush(stdout);
        strcpy(memory->buffer,"Child was here!\n\0");
        // 5) Give semaphore
        if (sem_post(semaphore)<0)
            perror("Sem get was not succ");
    }


    return 0;
}