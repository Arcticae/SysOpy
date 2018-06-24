#define _GNU_SOURCE

#include <signal.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_t main_thread;
pthread_t side_thread;
pthread_mutex_t mutex;
void sigint_handler(int signo){
    printf("Succesfully exited thread\n");
    sigval_t values;
    values.sival_int=69; // XDXDX DATS FUNNEH
    values.sival_ptr=NULL;
    pthread_sigqueue(main_thread,SIGUSR1,values);
    pthread_exit(NULL);
    //before death. we send the signal of value 69 to main thread.
}
void sigusr1_handler(int signo,siginfo_t*siginfo,void*ignore_this){
    printf("Main thread got a signal from child %d\n", siginfo->si_value.sival_int);
    pthread_exit(NULL);
}


void *thread_task(void *args) {
    pthread_detach(pthread_self());
    signal(SIGINT,sigint_handler);
    while(1){
        printf("This is pthread task\n");
        sleep(5);
    }

}

int main(int argc, char **argv) {
    main_thread=pthread_self();
    struct sigaction sigaction1;
    sigaction1.sa_flags=SA_SIGINFO;
    sigaction1.sa_sigaction=sigusr1_handler;
    sigemptyset(&sigaction1.sa_mask);
    sigaddset(&sigaction1.sa_mask,SIGUSR2);
    sigaction(SIGUSR1,&sigaction1,NULL);
    // 1 ) Create attributes for thread and initialize them
    pthread_attr_t attributes;
    pthread_attr_init(&attributes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
    // 2 ) Thread Create and check if equal
    pthread_create(&side_thread,&attributes,thread_task,NULL);
    pthread_attr_destroy(&attributes);
    printf("Threads equal?: %d\n",pthread_equal(main_thread, side_thread));

    while(1){}

    //MUTEXES



        return 0;
}