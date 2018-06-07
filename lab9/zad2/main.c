#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>

void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}


FILE *source;
int P, K, N, L, seek_mode, nk, communicates_enable;
char file_path[FILENAME_MAX];

pthread_t *producer_threads;
pthread_t *consumer_threads;
int produced = 0;
int consumed = 0;
sem_t*semaphores;
char **buffer;
int EndOfFile=0;

void handle_interrupt(int sig) {       //after NK secs there is a alert, or cancellation due to ctrl+c
    if(sig==SIGALRM)printf("Timeout,stopping program.\n");
    int i;
    for (i = 0; i < P; i++)
        pthread_cancel(producer_threads[i]);
    for (i = 0; i < K; i++)
        pthread_cancel(consumer_threads[i]);
    exit(EXIT_SUCCESS);
}

void init_stats(char *filepath) {

    FILE *file = fopen(filepath, "r");
    if (file == NULL)
        failure("Error opening file:");

    fscanf(file, "%d %d %d %s %d %d %d %d", &P, &K, &N, file_path, &L, &seek_mode, &communicates_enable, &nk);
    printf("Run config:\nP: %d\nK: %d\nN: %d\nPath: %s\nL: %d\nSeek Mode: %d\nCommunicates enabled: %d\nNK param: %d\n ",
           P, K, N, file_path, L, seek_mode, communicates_enable, nk);
    fclose(file);

}


void init_pthread() {
    int i;
    if(nk>0)signal(SIGALRM, handle_interrupt);
    signal(SIGTERM, handle_interrupt);       //same reaction

    source = fopen(file_path, "r");
    if (source == NULL)
        failure("There was an error opening pan tadek");

    semaphores = malloc((N + 3) * sizeof(sem_t));
    for (i = 0; i < N + 2; ++i)
        sem_init(&semaphores[i], 0, 1);
    sem_init(&semaphores[N+2], 0, (unsigned int) N);


    buffer = malloc(N * sizeof(char *));
    for (i = 0; i < N; i++) {
        buffer[i] = NULL;
    }

    consumer_threads = malloc(K * sizeof(pthread_t));
    producer_threads = malloc(P * sizeof(pthread_t));

}

void delete_pthread() {
    int i;

    for (i = 0; i < N; i++) {
        if (buffer[i] != NULL)free(buffer[i]);
    }
    free(buffer);
    for (i = 0; i < N + 4; ++i)
        sem_destroy(&semaphores[i]);
    free(semaphores);

    fclose(source);
}

int is_wanted(int line_length){
    return seek_mode == (line_length > L ? 1 : line_length < L ? -1 : 0);
}
void *producer_task(void *args) {
    int i;
    char line_buffer[1024];
    while (fgets(line_buffer, 1024, source) != NULL) {
        if(communicates_enable)printf("Producer: %ld |\t Getting a line from a file\n",pthread_self());

        sem_wait(&semaphores[N]);

        sem_wait(&semaphores[N+2]);

        i = produced;
        if(communicates_enable)printf("Producer: %ld |\t Acquired an index %d\n",pthread_self(),i);
        produced = (produced + 1) % N;      //this index is already taken. take mutex of the index to insert data

        sem_wait(&semaphores[i]);
        sem_post(&semaphores[N]);

        buffer[i] = malloc((strlen(line_buffer) + 1) * sizeof(char));
        strcpy(buffer[i], line_buffer);
        if(communicates_enable)printf("Producer: %ld |\t Put a line at %d\n",pthread_self(),i);

        sem_post(&semaphores[i]);
    }
    if(communicates_enable)printf("Producer: %ld |\t Finished work\n",pthread_self());
    return NULL;
}

void *consumer_task(void *args) {
    int i;
    char *line_buffer;

    while (1) {
        sem_wait(&semaphores[N+1]);

        while (buffer[consumed] == NULL) {
            sem_post(&semaphores[N+1]);
            if(EndOfFile){
                if(communicates_enable)printf("Consumer: %ld |\t End of content. \n",pthread_self());
                return NULL;
            }

            sem_wait(&semaphores[N+1]);
        }

        i = consumed;
        consumed = (consumed + 1) % N;

        if(communicates_enable)printf("Consumer: %ld |\t Acquired an index %d\n",pthread_self(),i);

        sem_wait(&semaphores[i]);;

        line_buffer=buffer[i];
        buffer[i]=NULL;     //consume
        if(communicates_enable)printf("Consumer: %ld |\t Consumed a line at %d\n",pthread_self(),i);
        //printf("%s",line_buffer);
        //fflush(stdout);
        if(is_wanted((int)strlen(line_buffer))){
            printf("Consumer: %ld | Got a match for your seek requirements: \n%s ",pthread_self(),line_buffer);
        }
        sem_post(&semaphores[N+2]);
        sem_post(&semaphores[N + 1]);
        sem_post(&semaphores[i]);

    }
}

void run_threads() {
    int i;
    for (i = 0; i < P; i++) {
        pthread_create(&producer_threads[i], NULL, producer_task, NULL);
        if(communicates_enable)printf("Producer: %ld |\t CREATED\n",producer_threads[i]);
    }
    for (i = 0; i < K; i++)
        pthread_create(&consumer_threads[i], NULL, consumer_task, NULL);

    if (nk > 0)alarm((unsigned) nk); //start measuring time
}

void join_threads() {
    int i;
    for (i = 0; i < P; i++)
        pthread_join(producer_threads[i], NULL);
    EndOfFile=1;

    for (i = 0; i < K; i++)
        pthread_join(consumer_threads[i], NULL);

}

int main(int argc, char **argv) {

    if (argc < 2)
        failure("Not sufficient arguments given, give me filepath to configuration file");
    init_stats(argv[1]);

    init_pthread();
    run_threads();
    join_threads();
    delete_pthread();

    return 0;
}