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
int shm_fd;
shared_seg *memory;

void tear_down(int sig) {
    sem_close(semaphore);
    sem_unlink("/semaforka");
    munmap(memory,sizeof(shared_seg));
    shm_unlink("/shm_posix");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    signal(SIGINT, tear_down);
    signal(SIGTSTP, tear_down);
    atexit(tear_down);
    //SEMAPHORES
    //POSIX


    // 1sem) Open sem
    semaphore = sem_open("/semaforka", O_CREAT | O_EXCL, S_IRWXU, 1);

    shm_fd = shm_open("/shm_posix", O_CREAT | O_EXCL | O_RDWR , S_IRWXU);

    ftruncate(shm_fd,sizeof(shared_seg));

    memory = mmap(NULL, sizeof(shared_seg), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);



    while (1) {
        // 2) Get semaphore
        if (sem_wait(semaphore) < 0)
            perror("Sem get was not succ");

        printf("Parent got the semaphore! Previous content: %s\n", memory->buffer);
        fflush(stdout);
        strcpy(memory->buffer,"Parent was here!\n\0");
        // 3) Give semaphore
        if (sem_post(semaphore) < 0)
            perror("Sem get was not succ");
    }


    return 0;
}