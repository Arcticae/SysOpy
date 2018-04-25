//
// Created by timelock on 25.04.18.
//

#ifndef LAB6_QUEUEDETAILS_H
#define LAB6_QUEUEDETAILS_H
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE

#include <sys/ipc.h>

#define MIRROR 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define TIME 6
#define END 7
#define INTRODUCE 8       //client communicate, defines when wanted to be registered.
#define GOODBYE 9        //client communicate, defines when exited.

#define MAX_MESSAGE_SIZE 1024
#define MESSAGE_SIZE sizeof(struct msgbuffer_new)
#define MAX_CLIENTS 9
#define MAX_MESSAGE_AMOUNT 9
#define SRV_DIRECTORY "/serv"


struct msgbuffer_new{
    long mtype;
    int index;
    char message[MAX_MESSAGE_SIZE];
};


#endif //LAB6_QUEUEDETAILS_H
