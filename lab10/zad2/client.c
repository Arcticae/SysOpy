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

char *client_name;
char*self_unix_path="/socket-self";
enum connection_type con_type;
char*unix_path;


void tear_down(int sig) {    //also a procedure for ctrl+c
    if (unlink(self_unix_path) == -1)
        failure("Deleting self endpoint not succesful");
    send_header(SIGNOUT,0,0);
    if (close(socket_fd) == -1) {
        failure("Removing local socket was not succesful");
    }
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
            strtok(srv_address, ":");
            if ((port = strtok(NULL, ":")) == NULL)
                failure("Not able to retrieve port from the given address");
            con_type=INET;

            if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
                failure("Could not create socket");

            int portnum = (int) strtol(port, NULL, 10);

            if (portnum < 1024 || portnum > 65535)
                failure("There is no way you can use the privileged port");
            struct sockaddr_in ipv4_address;

            ipv4_address.sin_family = AF_INET;
            ipv4_address.sin_addr.s_addr=htonl(INADDR_ANY);
            ipv4_address.sin_port = 0;
            if (connect(socket_fd, &ipv4_address, sizeof(ipv4_address)) == -1)
                failure("Connection to server failed");

            ipv4_address.sin_addr.s_addr=htonl(inet_addr(srv_address));
            ipv4_address.sin_port = htons(portnum);
            if (connect(socket_fd, &ipv4_address, sizeof(ipv4_address)) == -1)
                failure("Connection to server failed");

            break;
        }
        case AF_UNIX: {
            unix_path=srv_address;

            con_type=UNIX;

            //srv_address in this case corresponds to pathname
            if (strlen(unix_path) < 1 || strlen(unix_path) > UNIX_PATH_MAX)
                failure("Correctn't pathlength for afunix");

            struct sockaddr_un server_address;
            struct sockaddr_un client_address;
            memset(&server_address,0,sizeof(server_address));
            memset(&client_address,0,sizeof(client_address));
            server_address.sun_family = AF_UNIX;
            client_address.sun_family = AF_UNIX;
            snprintf(server_address.sun_path,UNIX_PATH_MAX, "%s", unix_path);
            snprintf(client_address.sun_path,UNIX_PATH_MAX,"%s",self_unix_path);

            if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0))<0)
                failure("Could not create socket");


            if(bind(socket_fd,(const struct sockaddr*) &client_address,sizeof(client_address)))
                failure("Binding address failed");

            if(connect(socket_fd,(const struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
                failure("Connection to server failed");
            }


            break;
        }
        default:
            failure("Not recognised family address, counter \"AF_UNIX\"  or \"AF_INET\" \n");

    }
    //end init socket
    printf("Sockets initialized\n");
}

void send_header(int type,int counter,double val) {     //artificial frame-to-stream component, structure of communicate is: 1)Type 2)Size 3)Who
    message msg;
    msg.type=(enum command_type) type;
    snprintf(msg.login,UNIX_PATH_MAX,"%s",client_name);
    msg.con_type=con_type;
    msg.counter=counter;
    msg.value=val;
    if(write(socket_fd,&msg,sizeof(message))!=sizeof(message))
        failure("Sending message was not succesful");

}

void send_registry_msg() {

    send_header(REGISTER,0,0);

    int response;
    printf("Waiting for response...\n");
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
    char command_line_cmd[256];

    if(read(socket_fd, &task, sizeof(task)) != sizeof(task))
        failure("Couldn't get the task from socket");

    int counter=task.counter;
    double val=0;

    sprintf(command_line_cmd,"echo 'scale=6; %lf %c %lf' | bc",task.arg1,task.operand,task.arg2);
    FILE*task_calc=popen(command_line_cmd,"r");
    size_t read_bytes=fread(command_line_cmd,1,256,task_calc);
    pclose(task_calc);
    command_line_cmd[read_bytes-1]='\0';
    sscanf(command_line_cmd,"%lf",&val);

    send_header(REPLY,counter,val);

}

void process_tasks(){
    int type;

    while(1){
        if(read(socket_fd,&type,sizeof(type))!=sizeof(type))
            failure("Reading msgtype not succesful from server");
        switch (type){
            case CALC:{
                handle_calc();
                break;
            }
            case PING:{
                send_header(PONG,0,0);
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