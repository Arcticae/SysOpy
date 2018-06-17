//
// Created by timelock on 16.06.18.
//

#ifndef LAB10_COMMON_H
#define LAB10_COMMON_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UNIX_PATH_MAX 108
#define MAX_CLIENTS 20      //10 per socket

typedef enum command_type{
    PING,
    PONG,
    REGISTER,
    REGISTERED,
    NAME_TAKEN,
    SERVER_FULL,
    SIGNOUT,
    REPLY,
    CALC

}command_type;

typedef struct command{
    int counter;
    double arg1,arg2;
    char operand;
}command;

typedef struct reply{
    int counter;
    double result;
}reply;

typedef struct client{
    int socket;
    char*login;
    int is_active;
}client;


#endif //LAB10_COMMON_H
