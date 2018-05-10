//
// Created by timelock on 25.04.18.
//
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "queuedetails.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <mqueue.h>
#include <sys/stat.h>

int end_received = 0;

mqd_t server_queue;
mqd_t client_queues[MAX_CLIENTS];
ssize_t ercode;
int current_clients = 0;

void handle_interrupt(int sig) {
    exit(0);
}

void exit_procedure() {

    mq_close(server_queue);
    mq_unlink(SRV_DIRECTORY);

    current_clients--;
    while (current_clients > 0) {
        mq_close(client_queues[current_clients]);
        current_clients--;
    }
}

void start_server() {

    atexit(exit_procedure);                        //setting procedures for exit and sigs
    signal(SIGINT, handle_interrupt);

    struct mq_attr server_attributes;
    server_attributes.mq_msgsize = MESSAGE_SIZE;
    server_attributes.mq_maxmsg = 10;

    server_queue = mq_open(SRV_DIRECTORY, O_RDONLY | O_CREAT | O_EXCL , S_IRWXU | S_IRWXG, &server_attributes);

    if (server_queue == -1) {
        perror("Error creating queue 4 serv");
        exit(EXIT_FAILURE);
    }
}

void send_message(mqd_t queue_id, struct msgbuffer_new *message) {
    mq_send(queue_id, (char *) message, MESSAGE_SIZE, 0);
    
}



void handle_INTRODUCE(struct msgbuffer_new *message, struct msgbuffer_new *response);

void handle_CALC(struct msgbuffer_new *message, struct msgbuffer_new *response, int OPCODE);

void handle_TIME(struct msgbuffer_new *message, struct msgbuffer_new *response);

void handle_MIRROR(struct msgbuffer_new *message, struct msgbuffer_new *response);

void remove_client(struct msgbuffer_new *message, struct msgbuffer_new *response);

int main(int argc, char **argv) {

    start_server();
    while (1) {
        struct msgbuffer_new message;
        struct msgbuffer_new response;

        if (mq_receive(server_queue, (char *) &message, MESSAGE_SIZE, 0) ==-1) {      //we do not wait for the communicate, because we have infinite loop
            if (errno != EAGAIN) {
                perror("Error while fetching message from srv queue");
                exit(EXIT_FAILURE);
            } else if (end_received == 1)exit(EXIT_SUCCESS);

        } else {
            printf("I have received taskwith args: %s\tid:%d\topcode:%lo\n", message.message, message.index,
                   message.mtype);

            switch (message.mtype) {
                case INTRODUCE:
                    handle_INTRODUCE(&message, &response);
                    break;
                case ADD:
                    handle_CALC(&message, &response, ADD);
                    break;
                case SUB:
                    handle_CALC(&message, &response, SUB);
                    break;
                case MUL:
                    handle_CALC(&message, &response, MUL);
                    break;
                case DIV:
                    handle_CALC(&message, &response, DIV);
                    break;
                case TIME:
                    handle_TIME(&message, &response);
                    break;
                case MIRROR:
                    handle_MIRROR(&message, &response);
                    break;
                case END:
                    end_received = 1;
                    break;
                case GOODBYE:
                    remove_client(&message, &response);
                    continue;
                default:
                    printf("Unnknown signal from sb\n");
                    continue;


            }
            //respond after doing stuff
            response.mtype = message.mtype;
            send_message(client_queues[response.index],&response);
        }

    }
}


void handle_INTRODUCE(struct msgbuffer_new *message, struct msgbuffer_new *response) {
    if (current_clients < MAX_CLIENTS) {

        client_queues[current_clients] = mq_open(message->message, O_WRONLY);

        if (client_queues[current_clients] == -1) {
            perror("Cannot get client's queue");
            exit(EXIT_FAILURE);
        }

        response->mtype = INTRODUCE;
        response->index = current_clients;
        current_clients++;

    } else {
        fprintf(stderr, "Max number of clients exceeded\nExiting...");
        exit(EXIT_FAILURE);
    }
}

void handle_CALC(struct msgbuffer_new *message, struct msgbuffer_new *response, int OPCODE) {
    int n1, n2;
    char *c1, *c2;
    if ((c1 = strtok(message->message, " \t\n")) == NULL || (c2 = strtok(NULL, " \n\t")) == NULL) {
        sprintf(response->message, "Wrong format of arguments given by you.\n");

    } else {
        int res;
        n1 = (int) strtol(c1, NULL, 10);
        n2 = (int) strtol(c2, NULL, 10);
        switch (OPCODE) {
            case ADD:
                res = n1 + n2;
                break;
            case SUB:
                res = n1 - n2;
                break;
            case MUL:
                res = n1 * n2;
                break;
            case DIV:
                res = n1 / n2;
                break;
            default:
                fprintf(stderr, "No such operation, error.");
                res = 0;
                break;
        }
        sprintf(response->message, "%d", res);
    }
}

void handle_TIME(struct msgbuffer_new *message, struct msgbuffer_new *response) {
    struct timeval timeBuffer;
    gettimeofday(&timeBuffer, NULL);
    struct tm *timestruct = localtime(&timeBuffer.tv_sec);
    strftime(response->message, MESSAGE_SIZE, "%Y-%m-%d %H:%M:%S", timestruct);
}

void handle_MIRROR(struct msgbuffer_new *message, struct msgbuffer_new *response) {
    size_t len = strlen(message->message);
    int i;
    for (i = 0; i < len; i++) {
        response->message[len - 1 - i] = message->message[i];
    }
    response->message[len] = '\0';

}

void remove_client(struct msgbuffer_new *message, struct msgbuffer_new *response) {

    int i;
    for (i = message->index; i < current_clients; i++) {
        client_queues[i] = client_queues[i + 1];
    }
    current_clients--;
}
