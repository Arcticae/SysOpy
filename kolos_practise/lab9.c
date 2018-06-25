#define _GNU_SOURCE

#include <signal.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>


pthread_t second_thread;
pthread_mutex_t mutex;

void* thread_task(void*some_data){

}

int main(int argc, char **argv) {
    // 1) Attribute handling

    pthread_mutexattr_t attributes;

    // Init attributes

    pthread_mutexattr_init(&attributes);

    // You can set type of mutex

    pthread_mutexattr_settype(&attributes,PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_settype(&attributes,PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_settype(&attributes,PTHREAD_MUTEX_FAST_NP);

    // You can also check the type of mutex

    int typeofmutex;
    pthread_mutexattr_gettype(&attributes,&typeofmutex);

    // We can set/get the protocol to entering the mutex with different priorities : PTHREAD_PROCESS_SHARED is 1, PTHREAD_PROCESS_PRIVATE is 0

    pthread_mutexattr_setpshared(&attributes,PTHREAD_PROCESS_SHARED);

    // And also inheriting

    pthread_mutexattr_setprotocol(&attributes,PTHREAD_PRIO_NONE);
    pthread_mutexattr_setprotocol(&attributes,PTHREAD_PRIO_INHERIT);
    pthread_mutexattr_setprotocol(&attributes,PTHREAD_PRIO_PROTECT);

    // 1) Doing the realstuff with mutex

    pthread_mutex_init(&mutex,&attributes);
    pthread_mutex_lock(&mutex);
    pthread_mutex_trylock(&mutex);
    pthread_mutex_unlock(&mutex);

    // 2 ) Unnamed sems

    sem_t semaphore;

    sem_init(&semaphore,PTHREAD_PROCESS_SHARED,1);
    sem_post(&semaphore);
    sem_wait(&semaphore);
    sem_trywait(&semaphore);
    sem_destroy(&semaphore);

    // 3 ) Condition variables

    pthread_cond_t condition_var;
    pthread_condattr_t condition_attributes;
    //Initialize attributes and cond
    pthread_condattr_setpshared(&condition_attributes,PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&condition_attributes,PTHREAD_PROCESS_PRIVATE);
    pthread_cond_init(&condition_var,&condition_attributes);
    //Wait for the conditon
    int i=1;
    // Procedure -> First, get the mutex and check it.
    pthread_mutex_lock(&mutex);
    while(i!=0){
    // Then -> You need to wait for it and bind the mutex to the condvar so it gets taken when condition is true
        pthread_cond_wait(&condition_var,&mutex);
    }
    //We do stuff (might change the ival or not, so we notify that one should check the condition)
    pthread_cond_broadcast(&condition_var);
    // Then we unlock the mutex
    pthread_mutex_unlock(&mutex);
    // And destroy it.
    pthread_cond_destroy(&condition_var);

    return 0;
}