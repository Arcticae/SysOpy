#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}


FILE *source;
int P, K, N, L, seek_mode, nk, communicates_enable;
char file_path[FILENAME_MAX];

pthread_t *producer_threads;
pthread_t *consumer_threads;
//version with monitor -> 2 ints to monitor production and consumption
int produced = 0;       //these will need guarded conditions
int consumed = 0;       //these will need guarded conditions (i think)
pthread_cond_t queue_full;
pthread_cond_t queue_empty;


char **buffer;
pthread_mutex_t *mutexes;
pthread_mutex_t producer_consumer_mutex[2];

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

    if(nk>0)signal(SIGALRM, handle_interrupt);
    signal(SIGTERM, handle_interrupt);       //same reaction

    source = fopen(file_path, "r");
    if (source == NULL)
        failure("There was an error opening pan tadek");

    mutexes = malloc(N * sizeof(pthread_mutex_t));
    int i;
    for (i = 0; i < N; i++) {
        pthread_mutex_init(&mutexes[i], NULL);    //regular mutexes for every cell
    }

    pthread_mutex_init(&producer_consumer_mutex[0], NULL);
    pthread_mutex_init(&producer_consumer_mutex[1], NULL);
    pthread_cond_init(&queue_full, NULL);
    pthread_cond_init(&queue_empty, NULL);

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

    for (i = 0; i < N; i++) {
        pthread_mutex_destroy(&mutexes[i]);
    }
    free(mutexes);

    pthread_cond_destroy(&queue_empty);
    pthread_cond_destroy(&queue_full);

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
        pthread_mutex_lock(&producer_consumer_mutex[0]);    //get mutex to access write amongst producers :)

        while (buffer[produced] != NULL){
            pthread_cond_wait(&queue_full,&producer_consumer_mutex[0]); //the queue is full, give producers mutex and wait for free space to fill
            if(communicates_enable)printf("Producer: %ld |\t QUEUE OVERFLOW,WAITING FOR CONSUMER \n",pthread_self());
        }

        i = produced;
        if(communicates_enable)printf("Producer: %ld |\t Acquired an index %d\n",pthread_self(),i);
        produced = (produced + 1) % N;      //this index is already taken. take mutex of the index to insert data

        pthread_mutex_lock(&mutexes[i]);

        buffer[i] = malloc((strlen(line_buffer) + 1) * sizeof(char));
        strcpy(buffer[i], line_buffer);
        if(communicates_enable)printf("Producer: %ld |\t Put a line at %d\n",pthread_self(),i);

        pthread_cond_broadcast(&queue_empty);                //condition may have changed for queue not having enough elems. let consumers know we have the good stuff back B-)
        pthread_mutex_unlock(&producer_consumer_mutex[0]);  //unlock the mutexes so producers and consumers can have them
        pthread_mutex_unlock(&mutexes[i]);
    }
    if(communicates_enable)printf("Producer: %ld |\t Finished work\n",pthread_self());
    return NULL;
}

void *consumer_task(void *args) {
    int i;
    char *line_buffer;

    while (1) {
        pthread_mutex_lock(&producer_consumer_mutex[1]);

        while (buffer[consumed] == NULL) {
            if(EndOfFile){
                pthread_mutex_unlock(&producer_consumer_mutex[1]);
                if(communicates_enable)printf("Consumer: %ld |\t End of content. \n",pthread_self());
                return NULL;
            }

            pthread_cond_wait(&queue_empty,&producer_consumer_mutex[1] );
        }

        i = consumed;
        consumed = (consumed + 1) % N;

        if(communicates_enable)printf("Consumer: %ld |\t Acquired an index %d\n",pthread_self(),i);

        pthread_mutex_lock(&mutexes[i]);

        line_buffer=buffer[i];
        buffer[i]=NULL;     //consume
        if(communicates_enable)printf("Consumer: %ld |\t Consumed a line at %d\n",pthread_self(),i);
        //printf("%s",line_buffer);
        // fflush(stdout);
        if(is_wanted((int)strlen(line_buffer))){
            printf("Consumer: %ld | Got a match for your seek requirements: \n%s ",pthread_self(),line_buffer);
        }
        pthread_cond_broadcast(&queue_full);
        pthread_mutex_unlock(&mutexes[i]);
        pthread_mutex_unlock(&producer_consumer_mutex[1]);

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
    pthread_cond_broadcast(&queue_empty);

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