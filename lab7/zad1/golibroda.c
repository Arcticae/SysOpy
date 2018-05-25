#include <signal.h>
#include "shared.h"

unsigned max_clients;

int semgroup_id;
int shared_memory_id;

void sig_handle(int sig) {
    printf("Golibroda exiting...\n");
    fflush(stdout);
    exit(0);
}

void exit_procedure() {
    semctl(semgroup_id,0,IPC_RMID);
    shmctl(shared_memory_id,IPC_RMID,NULL);
}

void setup_golibroda() {

    signal(SIGKILL, sig_handle);
    signal(SIGTERM, sig_handle);
    signal(SIGTSTP, sig_handle);
    atexit(exit_procedure);
    key_t shared_key = ftok(getenv("HOME"), QUEUE_KEY);
    if (shared_key == -1) {
        perror("Getting ftok was unsuccsesful");
        exit(EXIT_FAILURE);
    }

    shared_memory_id = shmget(shared_key, sizeof(struct barber_info), S_IRWXU | IPC_CREAT);

    if (shared_memory_id == -1) {
        perror("Getting shmid was unsuccsesful");
        exit(EXIT_FAILURE);
    }
    barber = shmat(shared_memory_id, 0, 0);
    if (barber == (void *) -1) {
        perror("Cannot access shm");
        exit(EXIT_FAILURE);
    }

    if ((semgroup_id = semget(shared_key, 1, S_IRWXU | IPC_CREAT)) == -1) {
        perror("Cannot get sems");
        exit(EXIT_FAILURE);
    }

    int i;
    semctl(semgroup_id,0,SETVAL,0);
    barber->barber_status=SLEEPING;
    barber->clients=0;
    barber->chair=0;
    barber->queue_size=max_clients;
    for(i=0;i<barber->queue_size;i++)barber->fifo_queue[i]=0;
}

void shave(){
    printf("Timestamp: %ld | Golibroda started cutting of cli no: %d\n", get_time(), barber->chair);
    fflush(stdout);
    printf("Timestamp: %ld | Golibroda finished cutting of cli no: %d\n", get_time(), barber->chair);
    fflush(stdout);
    barber->chair=0;
}

void invite_client(){
    barber->chair=barber->fifo_queue[0];
    printf("Timestamp: %ld | Golibroda invited cli no: %d\n", get_time(), barber->chair);
    fflush(stdout);
}



int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Specify the queue capacity\n");
        exit(EXIT_FAILURE);
    }
    max_clients = (unsigned) strtol(argv[1], NULL, 10);
    if (max_clients < 1 || max_clients > MAXQUEUE_SIZE) {
        printf("Wrong arg. Try agian\n");
        exit(EXIT_FAILURE);
    }

    setup_golibroda();
    give_semaphore(semgroup_id);

    while(1){
        take_semaphore(semgroup_id);
        switch(barber->barber_status){
            case IDLE:
                if(queue_empty()){
                    printf("Timestamp: %ld | Golibroda fell asleep\n", get_time());
                    fflush(stdout);
                    barber->barber_status=SLEEPING;
                }else{
                    invite_client();
                    barber->barber_status=READY;
                }
                break;
            case AWAKEN:
                printf("Timestamp: %ld | Golibroda has awaken\n", get_time());
                fflush(stdout);
                barber->barber_status=READY;
                break;
            case SHAVING:
                shave();
                barber->barber_status=READY;
                break;
            default:
                break;
        }
        give_semaphore(semgroup_id);
    }

    return 0;
}

