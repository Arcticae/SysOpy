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

//Globals -> We need local path for AF_UNIX type
char *unix_path;
//Globals -> We need socket fd's for both types of connection
int ipv4_socket_fd;
int local_socket_fd;
//Globals -> We need global epoll var
int epoll;

void tear_down(int signo){
    close(ipv4_socket_fd);
    close(local_socket_fd);
    unlink(unix_path);
    close(epoll);


}

int main() {
    char *portnum = "4200";
    signal(SIGINT,tear_down);
    atexit(tear_down);
    // 1 ) How to setup IPV4 address

    //Get the socket
    ipv4_socket_fd = socket(AF_INET, SOCK_STREAM, 0);       //TODO -> protocol?

    //We need sockaddr structure to bind to.
    struct sockaddr_in ipv4sockaddr;
    //Port -> Host-to-net-short (HTONS)
    ipv4sockaddr.sin_port = htons((uint16_t) strtol(portnum, NULL, 10));
    //Address -> Host-to-net-long (HTONL)
    ipv4sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ipv4sockaddr.sin_family = AF_INET;

    //We bind the address to the socket made.

    if (bind(ipv4_socket_fd, &ipv4sockaddr, sizeof(struct sockaddr_in)) == -1)
        perror("error binding ipv4");

    if (listen(ipv4_socket_fd, 10) == -1)
        perror("Error listen to ipv4");
    //This connection now listens to 0.0.0.0:4200 for new clients.

    // 2 ) How to setup AF_UNIX address.

    local_socket_fd=socket(AF_UNIX,SOCK_STREAM,0);

    //same, get the unix address
    struct sockaddr_un unix_address;

    unix_address.sun_family=AF_UNIX;
    sprintf(unix_address.sun_path,"%s",unix_path);

    bind(local_socket_fd,&unix_address,sizeof(struct sockaddr_un));

    // Ok done. We now get the epoll epollin' B-)

    epoll=epoll_create1(0);
    struct epoll_event trigger; //what makes my epoll go epollin?
    trigger.events=EPOLLIN | EPOLLPRI;
    trigger.data.fd=-local_socket_fd;    //the epoll on this socket will return this on the event.
    epoll_ctl(epoll,EPOLL_CTL_ADD,local_socket_fd,&trigger);
    trigger.data.fd=-ipv4_socket_fd;
    epoll_ctl(epoll,EPOLL_CTL_ADD,ipv4_socket_fd,&trigger);
    // Both are monitored now with epoll. We can now perform wait for the signal on sockets.

    struct epoll_event epoll_get;
    int clients_socket;
    char buffer[256];

    while(1){
    epoll_wait(epoll, &epoll_get, 1, -1);  //-1 means wait inf for events.

    //Now the event has occured.
    //We can recognise the first connection or the other types by neg values
    if (epoll_get.data.fd < 0) {
       //If on listened port. Accept the connection.
        clients_socket=accept(-epoll_get.data.fd,NULL,NULL);
        //Add this to epoll
        trigger.data.fd=clients_socket;
        trigger.events=EPOLLIN;
    }
    else{
        read(epoll_get.data.fd,buffer,256);
        printf("Buffered msg: %s \n",buffer);
    }

    }
    return 0;
}