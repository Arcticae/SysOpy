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
#include <netdb.h>
#include "common.h"


int local_socket_fd;
int ipv4_socket_fd;

char *af_unix_path;
int current_command=1;
struct sockaddr_in ipv4_address;  //intet IPv4
struct sockaddr_un local_address;

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


void tear_down(int sig) {    //also a procedure for ctrl+c
    pthread_cancel(comm_thread);
    pthread_cancel(ping_thread);
    if (close(ipv4_socket_fd) == -1)
        failure("Closing ipv4 socket was not succesful");
    if (close(local_socket_fd) == -1)
        failure("Closing local socket was not succesful");
    if (unlink(af_unix_path) == -1)
        failure("Removing local socket was not succesful");
    if(close(epoll)==-1)
        failure("Closing epoll was not succesful");
}

void*ping_thread_task(void*args){
    int type=PING;
    int i;
    while(1){
        pthread_mutex_lock(&clients_mutex);
        for(i=0;i<current_clients;++i){
            if(!clients[i].is_active){
                printf("Client %s is unactive, removing the truten\n",clients[i].login);
                remove_client(clients[i].login,clients[i].socket);
                i--;
            } else{
                if(write(clients[i].socket,&type,sizeof(int))!=sizeof(int))
                    failure("Ping not succesful\n");
                clients[i].is_active=0;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(10);
    }
}
void *comm_thread_task(){
    srand((unsigned int) time(NULL));
    command task;
    int type=CALC;
    int err=0;
    char command_line[64];
    while(1){
        printf("Waiting for calc...\n");
        fgets(command_line,64,stdin);
        if(sscanf(command_line,"%lf %c %lf\n",&task.arg1,&task.operand,&task.arg2)!=3) {
            printf("Wrong format\nTry again\n");
            continue;
        }
        if(task.operand!='/'&&task.operand!='*'&&task.operand!='+'&&task.operand!='-') {
            printf("wrong operand inserted\nTry again\n");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if(current_clients==0){
            printf("No clients connected, unable to calculate the task");
            continue;
        }
        task.counter=current_command;

        err=0;
        int i=rand()%current_clients;
        if(write(clients[i].socket,&type,sizeof(type))!=sizeof(type))
            err=1;
        if(write(clients[i].socket,&task,sizeof(task))!=sizeof(task))
            err=1;
        if(err==1)
            printf("Could not send the request of calc\n");
        else
            printf("Succesfully sent request number %d of %lf %c %lf\n",current_command,task.arg1,task.operand,task.arg2);

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


    if ((ipv4_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        failure("Could not create inet socket");
    memset(&ipv4_address,0,sizeof(ipv4_address));
    ipv4_address.sin_family = AF_INET;
    ipv4_address.sin_addr.s_addr = htonl(INADDR_ANY);
    ipv4_address.sin_port = htons((uint16_t) portnum);

    if (bind(ipv4_socket_fd, &ipv4_address, sizeof(ipv4_address)) == -1)
        failure("Cannot bind the address to socket (ipv4)");

    if (listen(ipv4_socket_fd, 10) == -1)         //marking the socket as accepting connections
        failure("Listening to netport failed");
    printf("Address ipv4: %d\nPort ipv4: %d\n",ipv4_address.sin_addr,ipv4_address.sin_port);

    //ipv4 end init
    //local init
    if (strlen(local_path) < 1 || strlen(local_path) > UNIX_PATH_MAX)
        failure("Correctn't pathlength for afunix");

    if ((local_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
        failure("Could not create local socket");

    memset(&local_address,0,sizeof(local_address));
    sprintf(local_address.sun_path, "%s", local_path);
    local_address.sun_family = AF_UNIX;

    if (bind(local_socket_fd, &local_address, sizeof(local_address)) == -1)
        failure("Cannot bind the address to socket (local)");

    if (listen(local_socket_fd, 10) == -1)         //marking the socket as accepting connections
        failure("Listening to local failed");

    //local end init
    //epoll both sockets

    struct epoll_event trigger;
    trigger.events = EPOLLIN | EPOLLPRI;

    if ((epoll = epoll_create1(0)) == -1)
        failure("Creating epoll for both sockets was not succesful");

    trigger.data.fd = -local_socket_fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket_fd, &trigger) == -1)
        failure("Adding to epoll was not succesfull (local)");

    trigger.data.fd = -ipv4_socket_fd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, ipv4_socket_fd, &trigger) == -1)
        failure("Adding to epoll was not succesfull (ipv4)");

    if(pthread_create(&ping_thread,NULL,ping_thread_task,NULL)!=0)
        failure("Ping thread creating went wrong");

    if(pthread_create(&comm_thread,NULL,comm_thread_task,NULL)!=0)
        failure("Ping thread creating went wrong");
}

void connect_client(int socket) {

    int newsocket;

    if ((newsocket = accept(socket, NULL, NULL)) == -1)
        failure("Accepting connection from a socket went wrong");

    struct epoll_event trigger;
    trigger.events = EPOLLIN;
    trigger.data.fd = newsocket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, newsocket, &trigger) == -1)
        failure("Adding new socket to epol went wrong");


}

void remove_socket(int socket) {       //needs to be thread-protected

    //delete socket from being monitored and close it
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1)
        failure("Deleting socket from monitoring went wrong");
    if (shutdown(socket, SHUT_RDWR) == -1)
        failure("Shutting down communication failed");
    if (close(socket) == -1)
        failure("Closing socket failed");
    //end of closing proc

}

int name_taken(char *login) {       //thread-protected
    int i;
    for (i = 0; i < current_clients; i++) {
        if (strcmp(clients[i].login, login) == 0)
            return 1;
    }
    return 0;
}

void add_client(char *login, int socket) {
    int reply;
    pthread_mutex_lock(&clients_mutex);

    if (current_clients == MAX_CLIENTS) {
        reply = SERVER_FULL;
        if (write(socket, &reply, sizeof(reply)) == -1)
            failure("Sending off response failed");

        remove_socket(socket);
    }
    else {
        if (name_taken(login)){
            reply= NAME_TAKEN;
            if(write(socket,&reply,sizeof(reply))==-1)
                failure("Sending off response failed");

            remove_socket(socket);
        }
        else {
            clients[current_clients].socket=socket;
            clients[current_clients].is_active=1;
            clients[current_clients].login=malloc(strlen(login)+1);
            strcpy(clients[current_clients].login,login);

            reply = REGISTERED;
            if(write(socket,&reply,sizeof(reply))==-1)
                failure("Sending off response failed");
            printf("Registered client succesfully: %s\n",login);
            current_clients++;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(char*login,int socket) {     //needs to be thread_protected
    printf("Deleting client\n");
    pthread_mutex_lock(&clients_mutex);
    int is_present=0,i=0,j;
    for(i=0;i<current_clients;i++){
        if(strcmp(clients[i].login,login)==0) {
            is_present=1;
            break;
        }
    }
    if(is_present){
        remove_socket(socket);
        free(clients[i].login);
        current_clients--;
        for(j=i;j<current_clients;++j)
            clients[j]=clients[j+1];
    }
    pthread_mutex_unlock(&clients_mutex);

}

void receive_msg(int socket) {
    int type;
    int size;

    if (read(socket, &type, sizeof(int)) != sizeof(int))
        failure("Receiving header went wrong: counter");

    if (read(socket, &size, sizeof(int)) != sizeof(int))
        failure("Receiving header went wrong: size");
    char *who=malloc(size);

    if (read(socket, who, (size_t) size) != size)
        failure("Receiving login went wrong");

    switch (type) {
        case REGISTER: {
            add_client(who, socket);
            break;
        }
        case SIGNOUT: {
            remove_client(who, socket);
            break;
        }
        case REPLY: {
            reply response;
            if (read(socket, &response, sizeof(response)) == -1)
                failure("Receiving reply went wrong");
            else
                printf("%s says: Result is %lf\n", who, response.result);
            break;
        }
        case PONG: {        //needs to be thread safe

            pthread_mutex_lock(&clients_mutex);
            int is_present=0,i=0,j;
            for(i=0;i<current_clients;i++){
                if(strcmp(clients[i].login,who)==0) {
                    is_present=1;
                    break;
                }
            }
            if(is_present)
                clients[i].is_active=1;
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
        if (trigger.data.fd < 0)       //we know that the listening port gives values < 0
            connect_client(-trigger.data.fd);
        else
            receive_msg(trigger.data.fd);

    }
    tear_down(NULL);
    return 0;

}