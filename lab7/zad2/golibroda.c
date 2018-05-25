#include <signal.h>
#include "shared.h"

unsigned max_clients;

sem_t *sem;
int shared_memory_id;

void sig_handle(int sig) {
    printf("Golibroda exiting...\n");
    fflush(stdout);

    exit(0);
}

void exit_procedure() {
    if (sem_close(sem) == -1 || sem_unlink(keypath) == -1) {
        perror("Error detaching the sem");
        exit(EXIT_FAILURE);
    }
    if (munmap(barber, sizeof(barber)) == -1 || shm_unlink(keypath)==-1){
        perror("Error detaching the sem");
        exit(EXIT_FAILURE);
    }
    unlink("/dev/shm/sem.golibroda");
    unlink("/dev/shm/golibroda");
    printf("Detaching succesful\n");
}

void setup_golibroda() {

    signal(SIGKILL, sig_handle);
    signal(SIGTERM, sig_handle);
    signal(SIGTSTP, sig_handle);
    signal(SIGINT, sig_handle);
    atexit(exit_procedure);

    shared_memory_id = shm_open(keypath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);

    if (shared_memory_id == -1) {
        perror("Getting shmid was unsuccsesful");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_id, sizeof(*barber)) == -1) {
        perror("Trunc ntot succc");
        exit(EXIT_FAILURE);
    }
    if ((barber = (struct barber_info *) mmap(NULL, sizeof(struct barber_info), PROT_READ | PROT_WRITE, MAP_SHARED,
                                              shared_memory_id, 0)) == (struct barber_info *) -1) {
        perror("Cannot map barber info");
        exit(EXIT_FAILURE);
    }

    if ((sem = sem_open(keypath, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG, 0)) == (void *) -1) {
        perror("Cannot open semaphore");
        exit(EXIT_FAILURE);
    }

    int i;
    barber->barber_status = SLEEPING;
    barber->clients = 0;
    barber->chair = 0;
    barber->queue_size = max_clients;
    for (i = 0; i < barber->queue_size; i++)barber->fifo_queue[i] = 0;

}

void shave() {
    printf("Timestamp: %ld | Golibroda started cutting of cli no: %d\n", get_time(), barber->chair);
    fflush(stdout);
    printf("Timestamp: %ld | Golibroda finished cutting of cli no: %d\n", get_time(), barber->chair);
    fflush(stdout);
    barber->chair = 0;
}

void invite_client() {
    barber->chair = barber->fifo_queue[0];
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
    give_semaphore(sem);

    while (1) {
        take_semaphore(sem);
        switch (barber->barber_status) {
            case IDLE:
                if (queue_empty()) {
                    printf("Timestamp: %ld | Golibroda fell asleep\n", get_time());
                    fflush(stdout);
                    barber->barber_status = SLEEPING;
                } else {
                    invite_client();
                    barber->barber_status = READY;
                }
                break;
            case AWAKEN:
                printf("Timestamp: %ld | Golibroda has awaken\n", get_time());
                fflush(stdout);
                barber->barber_status = READY;
                break;
            case SHAVING:
                shave();
                barber->barber_status = READY;
                break;
            default:
                break;
        }
        give_semaphore(sem);
    }

    return 0;
}

