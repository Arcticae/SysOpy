//
// Created by timelock on 16.06.18.
//
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "common.h"


int local_socket_fd;
int ipv4_socket_fd;

char *af_unix_path;
int current_command = 1;

client clients[MAX_CLIENTS];
int current_clients = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int epoll;

pthread_t ping_thread;
pthread_t comm_thread;


void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}


void tear_down(int sig) {    //also a procedure for ctrl+c      //TODO
    pthread_cancel(comm_thread);
    pthread_cancel(ping_thread);
    if (close(ipv4_socket_fd) == -1)
        failure("Closing ipv4 socket was not succesful");
    if (close(local_socket_fd) == -1)
        failure("Closing local socket was not succesful");
    if (unlink(af_unix_path) == -1)
        failure("Removing local socket was not succesful");
    if (close(epoll) == -1)
        failure("Closing epoll was not succesful");
}

void *ping_thread_task(void *args) {
    int type = PING;
    int i;

    while (1) {
        sleep(5);
        pthread_mutex_lock(&clients_mutex);
        for (i = 0; i < current_clients; ++i) {
            if (!clients[i].is_active) {
                printf("Client %s is unactive, removing the truten\n", clients[i].login);
                remove_client(clients[i].login);
                i--;
            } else {
                int socket;

                if (clients[i].con_type == UNIX)socket = local_socket_fd;
                if (clients[i].con_type == INET)socket = ipv4_socket_fd;

                if (sendto(socket, &type, sizeof(type), 0, clients[i].sockaddress, clients[i].socketlength) !=
                    sizeof(type))
                    failure("Sending ping was failed");
                clients[i].is_active = 0;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(5);
    }
}

void *comm_thread_task(void*args) {
    srand((unsigned int) time(NULL));
    command task;
    int type = CALC;
    int err = 0;
    char command_line[64];
    while (1) {
        printf("Waiting for calc...\n");
        fgets(command_line, 64, stdin);
        if (sscanf(command_line, "%lf %c %lf\n", &task.arg1, &task.operand, &task.arg2) != 3) {
            printf("Wrong format\nTry again\n");
            continue;
        }
        if (task.operand != '/' && task.operand != '*' && task.operand != '+' && task.operand != '-') {
            printf("wrong operand inserted\nTry again\n");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (current_clients == 0) {
            printf("No clients connected, unable to calculate the task");
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        task.counter = current_command;

        err = 0;
        int i = rand() % current_clients;
        int socket;

        if (clients[i].con_type == UNIX)socket = local_socket_fd;
        if (clients[i].con_type == INET)socket = ipv4_socket_fd;

        if (sendto(socket, &type, sizeof(type), 0, clients[i].sockaddress, clients[i].socketlength) != sizeof(type))
            err = 1;
        if (sendto(socket, &task, sizeof(task), 0, clients[i].sockaddress, clients[i].socketlength) != sizeof(task))
            err = 1;
        if (err == 1) {
            printf("Sending failed\n");
            pthread_mutex_unlock(&clients_mutex);
            continue;
        } else
            printf("Succesfully sent request number %d of %lf %c %lf\n", current_command, task.arg1, task.operand,
                   task.arg2);

        current_command++;
        pthread_mutex_unlock(&clients_mutex);
    }

}

void set_up(int portnum, char *local_path) {

    //general init
    af_unix_path = local_path;
    signal(SIGINT, tear_down);
    atexit(tear_down);

    //ipv4 init
    if (portnum < 1024 || portnum > 65535)
        failure("There is no way you can use the privileged port");
    struct sockaddr_in ipv4_address;
    memset(&ipv4_address, 0, sizeof(ipv4_address));
    ipv4_address.sin_family = AF_INET;
    ipv4_address.sin_addr.s_addr = htonl(INADDR_ANY);
    ipv4_address.sin_port = htons((uint16_t) portnum);

    if ((ipv4_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        failure("Could not create inet socket");

    if (bind(ipv4_socket_fd, (const struct sockaddr*) &ipv4_address, sizeof(ipv4_address)) == -1)
        failure("Cannot bind the address to socket (ipv4)");


    //ipv4 end init
    //local init
    if (strlen(local_path) < 1 || strlen(local_path) > UNIX_PATH_MAX) {
        failure("Correctn't pathlength for afunix");
    }

    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;
    snprintf(local_address.sun_path, UNIX_PATH_MAX ,"%s", af_unix_path);


    if ((local_socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        failure("Could not create local socket");
    }


    if (bind(local_socket_fd, (const struct sockaddr*) &local_address, sizeof(local_address)))
        failure("Cannot bind the address to socket (local)");


    //local end init
    //epoll both sockets

    struct epoll_event trigger;
    trigger.events = EPOLLIN | EPOLLPRI;

    if ((epoll = epoll_create1(0)) == -1)
        failure("Creating epoll for both sockets was not succesful");

    trigger.data.fd = local_socket_fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket_fd, &trigger) == -1)
        failure("Adding to epoll was not succesfull (local)");

    trigger.data.fd = ipv4_socket_fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, ipv4_socket_fd, &trigger) == -1)
        failure("Adding to epoll was not succesfull (ipv4)");

    if (pthread_create(&ping_thread, NULL, ping_thread_task, NULL) != 0)
        failure("Ping thread creating went wrong");

    if (pthread_create(&comm_thread, NULL, comm_thread_task, NULL) != 0)
        failure("Ping thread creating went wrong");
}


int name_taken(char *login) {       //thread-protected
    int i;
    for (i = 0; i < current_clients; i++) {
        if (strcmp(clients[i].login, login) == 0)
            return 1;
    }
    return 0;
}

void add_client(int socket, struct sockaddr *sockaddr, socklen_t socklen, message msg) {
    int reply;
    pthread_mutex_lock(&clients_mutex);

    if (current_clients == MAX_CLIENTS) {
        reply = SERVER_FULL;
        if (sendto(socket, &reply, sizeof(reply), 0, sockaddr, socklen) != sizeof(reply))
            failure("Sending off response failed");
        free(sockaddr);
    } else {
        if (name_taken(msg.login)) {
            reply = NAME_TAKEN;
            if (sendto(socket, &reply, sizeof(reply), 0, sockaddr, socklen) != sizeof(reply))
                failure("Sending off response failed");
            free(sockaddr);
        } else {
            clients[current_clients].sockaddress = sockaddr;
            printf("Sockadrr:%s\n");
            clients[current_clients].con_type = msg.con_type;
            clients[current_clients].socketlength = socklen;
            clients[current_clients].login = malloc(strlen(msg.login) + 1);
            clients[current_clients].is_active = 1;
            strcpy(clients[current_clients].login, msg.login);

            reply = REGISTERED;
            if (sendto(socket, &reply, sizeof(reply), 0, sockaddr, socklen) != sizeof(reply))
                failure("Sending off response failed");
            printf("Registered client succesfully: %s\n", msg.login);
            current_clients++;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(char *login) {     //needs to be thread_protected
    printf("Deleting client %s\n", login);
    pthread_mutex_lock(&clients_mutex);
    int is_present = 0, i = 0, j;
    for (i = 0; i < current_clients; i++) {
        if (strcmp(clients[i].login, login) == 0) {
            is_present = 1;
            break;
        }
    }
    if (is_present) {
        free(clients[i].login);
        free(clients[i].sockaddress);
        current_clients--;
        for (j = i; j < current_clients; ++j)
            clients[j] = clients[j + 1];
    }
    pthread_mutex_unlock(&clients_mutex);

}

void receive_msg(int socket) {

    struct sockaddr *sockaddr = malloc(sizeof(struct sockaddr));
    socklen_t socklen = sizeof(struct sockaddr);
    message msg;

    if (recvfrom(socket, &msg, sizeof(message), 0, sockaddr, &socklen) != sizeof(message))
        failure("Cannot recvfrom the client");


    switch (msg.type) {
        case REGISTER: {
            printf("Registry message received: %s\n", msg.login);
            add_client(socket,sockaddr,socklen,msg);
            break;
        }
        case SIGNOUT: {
            remove_client(msg.login);
            break;
        }
        case REPLY: {
            printf("%s says: Result is %lf\n", msg.login, msg.value);
            break;
        }
        case PONG: {        //needs to be thread safe

            pthread_mutex_lock(&clients_mutex);
            int is_present = 0, i = 0;
            for (i = 0; i < current_clients; i++) {
                if (strcmp(clients[i].login, msg.login) == 0) {
                    is_present = 1;
                    break;
                }
            }
            if (is_present)
                clients[i].is_active = 1;
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        default:
            printf("Unrecognised message was received\n");

    }


}

int main(int argc, char **argv) {

    if (argc < 3)
        failure("Not proper number of args, give: \n1)Server port ex:\"4200\" \n2)Unix socket path\n ");

    set_up((int) strtol(argv[1], NULL, 10), argv[2]);

    struct epoll_event trigger;
    while (1) {

        if (epoll_wait(epoll, &trigger, 1, -1) == -1)
            failure("Epolling ports went wrong");
        receive_msg(trigger.data.fd);

    }
}