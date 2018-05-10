
#include "shared.h"
const char*queue_sem_path="/queue";
const char*check_sem_path="/check";
const char*asleep_sem_path="/asleep";
const char*clients_mem_path="/clients";

my_queue *clients = NULL;
unsigned max_clients;

sem_t *QUEUE;
sem_t *ASLEEP;
sem_t *CHECK;

void exit_procedure() {
    if (sem_close(ASLEEP) == -1 || sem_unlink(asleep_sem_path) == -1 ||
        sem_close(QUEUE) == -1 || sem_unlink(queue_sem_path) == -1 ||
        sem_close(CHECK) == -1 || sem_unlink(check_sem_path) == -1 ||
        munmap(clients, sizeof(clients)) == -1 ||
        shm_unlink(clients_mem_path) == -1
    ){
        perror("Golibroda: Closing resources failed");
        exit(EXIT_FAILURE);
    }

    printf("Timestamp: %ld | Golibroda: Detaching all resources complete.\n", get_time());
    fflush(stdout);
}

void setup_queue() {

    int shared_memory_id = shm_open( clients_mem_path , O_CREAT | O_TRUNC | O_RDWR , 0666);
    if (ftruncate(shared_memory_id, sizeof(my_queue)) == -1) {
        perror("Error trunc");
        exit(EXIT_FAILURE);
    }
    if ((clients = (my_queue *) mmap(NULL, sizeof(my_queue), PROT_READ | PROT_WRITE, MAP_SHARED,shared_memory_id,0))==(void*)-1){
        perror("Cannot map");
        exit(EXIT_FAILURE);
    }

    initialize_queue(clients, max_clients);
}

void setup_sems() {

    if ((ASLEEP = sem_open(asleep_sem_path, O_CREAT | O_TRUNC | O_RDWR, 0666, 0)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }
    if ((QUEUE = sem_open(queue_sem_path, O_CREAT | O_TRUNC | O_RDWR, 0666, 1)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }
    if ((CHECK = sem_open(check_sem_path, O_CREAT | O_TRUNC | O_RDWR, 0666, 1)) == SEM_FAILED) {
        perror("Failed to open a semaphore");
        exit(EXIT_FAILURE);
    }


}

pid_t check_client() {

    if(sem_wait(QUEUE)==-1){
        perror("Wait for QUEUE semaphore failed");
        exit(EXIT_FAILURE);
    }

    pid_t client_no = clients->chair;

    if(sem_post(QUEUE)==-1){
        perror("Giving of QUEUE semaphore failed");
        exit(EXIT_FAILURE);
    }

    return client_no;
}

void one_cut(pid_t pid) {
    printf("Timestamp: %ld | Golibroda: Before one cut of client no:%d\n", get_time(), pid);
    fflush(stdout);

    // kill(pid, SIGRTMIN);

    printf("Timestamp: %ld | Golibroda: After one cut of client no:%d\n", get_time(), pid);
    fflush(stdout);
}

void run_golibroda() {

    while (1) {

        if(sem_wait(ASLEEP)==-1){
            perror("Taking ASLEEP sem was failed");
            exit(EXIT_FAILURE);
        }
        if(sem_post(ASLEEP)==-1){
            perror("Posting ASLEEP sem was failed");
            exit(EXIT_FAILURE);
        }

        printf("Timestamp: %ld | Golibroda: I have been awaken!\n", get_time());
        pid_t next_customer = check_client();
        one_cut(next_customer);

        while (1) {

            if(sem_wait(QUEUE)==-1){
                perror("Taking QUEUE sem was failed");
                exit(EXIT_FAILURE);
            }

            next_customer = queue_pop(clients);

            if (next_customer != -1) {      //if queue is not empty. continue cutting.

                if(sem_post(QUEUE)==-1){
                    perror("Giving QUEUE sem was failed");
                    exit(EXIT_FAILURE);
                }

                printf("Timestamp: %ld | Golibroda: Inviting customer %d to be seated\n", get_time(), next_customer);
                one_cut(next_customer);

            } else {                     //else fall asleep and give semaphore
                printf("Timestamp: %ld | Golibroda: Falling asleep\n", get_time());
                fflush(stdout);
                if(sem_wait(ASLEEP)==-1){
                    perror("Taking ASLEEP sem was failed");
                    exit(EXIT_FAILURE);
                }
                if(sem_post(QUEUE)==-1){
                    perror("Giving QUEUE sem was failed");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }

    }
}
sigint_handle(int signum){
    //exit_procedure();
    exit(EXIT_FAILURE);
    return 0;
}
int main(int argc, char **argv) {

    if (argc < 4) {
        printf("Specify the queue capacity,wanted nubmer of clis, and cuts of a client in args plz\n");
        exit(EXIT_FAILURE);
    }

    max_clients = (unsigned) strtol(argv[1], NULL, 10);

    if (max_clients < 1 || max_clients > MAXQUEUE_SIZE) {
        printf("Wrong arg. Try agian\n");
        exit(EXIT_FAILURE);
    }

    atexit(exit_procedure);
    signal(SIGINT,sigint_handle);
    signal(SIGTSTP,sigint_handle);
    setup_queue();
    setup_sems();
    pid_t proc = fork();
    if (proc != 0) {
        printf("Golibroda PID: %d\n", getpid());
        printf("Timestamp: %ld | Golibroda: Initializing queue and sems complete. Golibroda is now asleep\n", get_time());
        fflush(stdout);

        run_golibroda();

        exit(EXIT_SUCCESS);
    } else execvp("./klienci", argv);

    while(1){

    }
    return 0;
}

