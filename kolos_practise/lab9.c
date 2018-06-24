#define _GNU_SOURCE

#include <signal.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex;
int main(int argc, char **argv) {

    pthread_mutexattr_t attributes;
    // 0) Init attributes
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr
    // 1) Init mutex

    pthread_mutex_init(&mutex,&attributes)
    pthread_mutex_lock(&mutex);
    pthread_mutex_trylock(&mutex);
    pthread_mutex_unlock(&mutex);
    return 0;
}