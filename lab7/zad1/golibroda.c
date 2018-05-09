#include <signal.h>
#include "shared.h"

my_queue *clients = NULL;
unsigned max_clients;

int semgroup_id;
int shared_memory_id;

key_t queue_key;


void exit_procedure() {
    if (shmdt(clients) == -1) {
        perror("Detaching mem not succesful");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shared_memory_id, IPC_RMID, NULL) == -1) {
        perror("Removeing ipc from memory not succ");
        exit(EXIT_FAILURE);
    }
    if (semctl(semgroup_id, 0, IPC_RMID) == -1) {
        perror("Rmving sms not succ");
        exit(EXIT_FAILURE);
    }
    printf("Timestamp: %ld | Golibroda: Detaching all resources complete.\n", get_time());
    fflush(stdout);
}

void setup_queue() {

    shared_memory_id = shmget(queue_key, sizeof(my_queue), IPC_CREAT | 0666);
    if (shared_memory_id == -1) {
        perror("Shared memory creation failed");
        exit(EXIT_FAILURE);
    }

    clients = (my_queue *) shmat(shared_memory_id, NULL, 0);

    if (clients == (my_queue *) -1) {
        perror("Attaching shared mem to address space of process has failed, reason");
        exit(EXIT_FAILURE);
    }

    initialize_queue(clients, max_clients);
}

void setup_sems() {

    semgroup_id = semget(queue_key, 4, IPC_CREAT | 0666);

    if (semgroup_id == -1) {
        perror("Getting semaphores failed, reason");
        exit(EXIT_FAILURE);
    }

    int i;


    for (i = 1; i < 3; i++) {

        if (semctl(semgroup_id, i, SETVAL, 1) == -1) {
            perror("Setting value for a semaphore has failed, reason");
            exit(EXIT_FAILURE);
        }

    }

    if (semctl(semgroup_id, 0, SETVAL, 0) == -1) {
        perror("Setting value for a semaphore has failed, reason");
        exit(EXIT_FAILURE);
    }

}

pid_t check_client(struct sembuf *action) {
    action->sem_num = QUEUE;
    action->sem_op = -1;

    if (semop(semgroup_id, action, 1) == -1) {
        perror("Taking QUEUE semaphore not succesful");
        exit(EXIT_FAILURE);
    }

    pid_t client_no = clients->chair;

    action->sem_op = 1;

    if (semop(semgroup_id, action, 1) == -1) {
        perror("Giving QUEUE semaphore not succesful");
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
    struct sembuf action;
    action.sem_flg = 0;

    while (1) {
        action.sem_op = -1;
        action.sem_num = AWAKE;

        if (semop(semgroup_id, &action, 1) == -1) {
            perror("Taking AWAKE semaphore not succesful");
            exit(EXIT_FAILURE);
        }
        printf("Timestamp: %ld | Golibroda: I have been awaken!\n", get_time());
        pid_t next_customer = check_client(&action);
        one_cut(next_customer);

        while (1) {
            action.sem_op = -1;
            action.sem_num = QUEUE;

            if (semop(semgroup_id, &action, 1) == -1) {
                perror("Taking QUEUE semaphore not succesful");
                exit(EXIT_FAILURE);
            }

            next_customer = queue_pop(clients);

            if (next_customer != -1) {      //if queue is not empty. continue cutting.
                action.sem_op = 1;

                if (semop(semgroup_id, &action, 1) == -1) {
                    perror("Giving QUEUE semaphore not succesful");
                    exit(EXIT_FAILURE);
                }
                printf("Timestamp: %ld | Golibroda: Inviting customer %d to be seated\n", get_time(),next_customer);
                one_cut(next_customer);

            } else {                     //else fall asleep and give semaphore
                printf("Timestamp: %ld | Golibroda: Falling asleep\n", get_time());
                fflush(stdout);
                action.sem_op = -1;
                action.sem_num = AWAKE;
                if (semop(semgroup_id, &action, 1) == -1) {
                    perror("Taking AWAKE semaphore not succesful");
                    exit(EXIT_FAILURE);
                }

                action.sem_op = 1;
                action.sem_num = QUEUE;
                if (semop(semgroup_id, &action, 1) == -1) {
                    perror("Giving QUEUE semaphore not succesful");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }

    }
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

    queue_key = QUEUE_KEY;

    atexit(exit_procedure);

    setup_queue();
    setup_sems();
    pid_t proc=fork();
    if(proc!=0){
        printf("Golibroda PID: %d\n",getpid());
        printf("Timestamp: %ld | Golibroda: Initializing queue and sems complete. Golibroda is now asleep\n", get_time());
        fflush(stdout);
        run_golibroda();
        exit(EXIT_SUCCESS);
    }
    else execvp("./klienci",argv);

    return 0;
}

