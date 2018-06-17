#define _BSD_SOURCE

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}

int socket_fd;
struct sockaddr_in ipv4_address;  //intet IPv4
struct sockaddr_un local_address;
char *client_name;

void tear_down(int sig) {    //also a procedure for ctrl+c

    send_header(SIGNOUT);
    if (shutdown(socket_fd, SHUT_RDWR) == -1)
        failure("Closing socket was not succesful");
    if (close(socket_fd) == -1)
        failure("Removing local socket was not succesful");
    printf("Shutting down client...\n");
    fflush(stdout);
}

void set_up(int addr_family, char *name, char *srv_address) {     //address full : ex "127.0.0.1:4200"
    //init general
    client_name = name;
    signal(SIGINT, tear_down);
    atexit(tear_down);
    //end init general

    //init socket
    switch (addr_family) {
        case AF_INET: {
            char *port;

            if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
                failure("Could not create socket");

            strtok(srv_address, ":");

            if ((port = strtok(NULL, ":")) == NULL)
                failure("Not able to retrieve port from the given address");


            int portnum = (int) strtol(port, NULL, 10);

            if (portnum < 1024 || portnum > 65535)
                failure("There is no way you can use the privileged port");

            ipv4_address.sin_family = AF_INET;
            ipv4_address.sin_port = htons(portnum);
            ipv4_address.sin_addr.s_addr=htonl(inet_addr(srv_address));

            if (connect(socket_fd, &ipv4_address, sizeof(ipv4_address)) == -1)
                failure("Connection to server failed");

            break;
        }
        case AF_UNIX: {
            //srv_address in this case corresponds to pathname
            if (strlen(srv_address) < 1 || strlen(srv_address) > UNIX_PATH_MAX)
                failure("Correctn't pathlength for afunix");

            local_address.sun_family = AF_UNIX;
            sprintf(local_address.sun_path, "%s", srv_address);

            if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
                failure("Could not create socket");

            if (connect(socket_fd, &local_address, sizeof(local_address)) == -1)
                failure("Connection to server failed");

            break;
        }
        default:
            failure("Not recognised family address, counter \"AF_UNIX\"  or \"AF_INET\" \n");

    }
    //end init socket
    printf("Connection succesful!\n");
}

void send_header(int type) {     //artificial frame-to-stream component, structure of communicate is: 1)Type 2)Size 3)Who
    int size = (int) (strlen(client_name)+1);
    if (write(socket_fd, &type, sizeof(int))!=sizeof(int))
        exit(EXIT_FAILURE);
    if (write(socket_fd, &size, sizeof(int))!=sizeof(int))
        failure("Failed to send message header to server");
    if (write(socket_fd, client_name,(size_t) size)!=size)
        failure("Failed to send message header to server");

}

void send_registry_msg() {

    send_header(REGISTER);

    int response;

    if(read(socket_fd,&response,sizeof(response))!=sizeof(response))
        failure("Waiting for response from server was unsuccsesful");

    switch (response){
        case SERVER_FULL: {
            failure("Server cannot register any more clients, try again later.\n");
            break;
        }
        case NAME_TAKEN: {
            failure("Name you registered under your client is already taken, restart the client with another name\n");
            break;
        }
        case REGISTERED: {
            printf("Succesfully registered under name: %s\n",client_name);
            break;
        }
        default:
            failure("Not recognised counter of response.\n");
    }

}

void handle_calc(){
    command task;
    reply reply;
    char command_line_cmd[256];

    if(read(socket_fd, &task, sizeof(task)) != sizeof(task))
        failure("Couldn't get the task from socket");

    reply.counter=task.counter;
    reply.result=0;

    sprintf(command_line_cmd,"echo 'scale=6; %lf %c %lf' | bc",task.arg1,task.operand,task.arg2);
    FILE*task_calc=popen(command_line_cmd,"r");
    size_t read_bytes=fread(command_line_cmd,1,256,task_calc);
    pclose(task_calc);
    command_line_cmd[read_bytes-1]='\0';
    sscanf(command_line_cmd,"%lf",&reply.result);

    send_header(REPLY);
    if(write(socket_fd,&reply,sizeof(reply))!=sizeof(reply))
        failure("Failed to reply to calc");

}

void process_tasks(){
    int type;

    while(1){
        if(read(socket_fd,&type,sizeof(int))!=sizeof(int))
            exit(EXIT_FAILURE);
        switch (type){
            case CALC:{
                handle_calc();
                break;
            }
            case PING:{
                send_header(PONG);
                break;
            }
            default:{
                printf("Not recognised counter of message\n");
                break;
            }
        }
    }
}


int main(int argc, char **argv) {

    if (argc < 3)
        failure("Not proper number of args, give: \n 1)Address family \"AF_UNIX\"  or \"AF_INET\"\n 2)Client name \n 3)Server address like \"127.0.0.1:4200\" ");

    int addr_family = (strcmp(argv[1], "AF_INET") == 0 ? AF_INET : strcmp(argv[1], "AF_UNIX") == 0 ? AF_UNIX : -1);

    if (addr_family == -1)
        failure("Not correct address family. AF_INET or AF_UNIX are accepted");

    set_up(addr_family, argv[2], argv[3]);

    send_registry_msg();

    process_tasks();

    return 0;
}