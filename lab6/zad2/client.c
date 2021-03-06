//
// Created by timelock on 25.04.18.
//
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE

#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include "queuedetails.h"

mqd_t server_queue;
mqd_t client_queue;
char*client_queue_name;
char server_queue_name;
int client_index;
int sent = 0;
int mother;
ssize_t ercode;

void handle_interrupt(int sig) {
	exit_procedure();
    printf("I have received signal SIGINT, terminating...\n");
    exit(0);
}

void receive_message(mqd_t queue_id, struct msgbuffer_new *message) {
    
        mq_receive(queue_id, (char *) message, MESSAGE_SIZE, 0);
        

}

void send_message(mqd_t queue_id, struct msgbuffer_new *message) {
        ercode = mq_send(queue_id, (char *) message, MESSAGE_SIZE, 0);
            
}

void exit_procedure() {


    mq_close(client_queue);        //remove queue
	mq_unlink(client_queue_name);

    if (mother == 1) {      //we need only to say goodbye when we know mother is alive.

        struct msgbuffer_new exit_message;
        exit_message.mtype = GOODBYE;
        exit_message.index = client_index;
        sprintf(exit_message.message, "CLNT_TERM");

        send_message(server_queue, &exit_message);

    }


}

void start_client() {

    atexit(exit_procedure);
    signal(SIGINT, handle_interrupt);

    struct mq_attr queue_attributes;
    queue_attributes.mq_msgsize = MESSAGE_SIZE;
    queue_attributes.mq_maxmsg = MAX_MESSAGE_AMOUNT;      //modifiable

    char queuepath[PATH_MAX] = {};
    sprintf(queuepath, "/%d", getpid());
    client_queue_name=queuepath;


    server_queue = mq_open(SRV_DIRECTORY, O_WRONLY );

    if (server_queue == -1) {
        perror("Error while opening queue for srv");
        exit(EXIT_FAILURE);
    }

    client_queue = mq_open(queuepath, O_RDONLY | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG,
                           &queue_attributes);

    if (client_queue == -1) {
        perror("Error while opening queue for clnt");
        exit(EXIT_FAILURE);
    }

}

void client_introduce() {
    struct msgbuffer_new message;
    struct msgbuffer_new response;

    message.mtype = INTRODUCE;
    strcpy(message.message, "INTRODUCE_CLNT");
    sprintf(message.message, "/%d", getpid());

    send_message(server_queue, &message);
    receive_message(client_queue, &response);

    client_index = response.index;     //for info where in array we are (del)

}

int main(int argc, char **argv) {

    if (argc < 1) {
        fprintf(stderr, "please give me filepath to interpret commands from\n");
        exit(EXIT_FAILURE);
    }

    FILE *batchfile;

    char *valid_strings[] = {"MIRROR", "ADD", "SUB", "MUL", "DIV", "TIME", "END"}; //needed 4 stuff



    if ((batchfile = fopen(argv[1], "r")) == NULL) {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }
    start_client();

    client_introduce();

    char buffer[MAX_MESSAGE_SIZE];
    char *temp_buffer;
    int i;
    mother = 1;

    while (fgets(buffer, MESSAGE_SIZE, batchfile)) {

        temp_buffer = strtok(buffer, " \n\t");  //we have in the buffer the name of command
        int valid = 0;
        struct msgbuffer_new message;

        for (i = 0; i < sizeof(valid_strings) / sizeof(char *) && temp_buffer != NULL; i++) {
            if (strcmp(valid_strings[i], temp_buffer) == 0) {

                valid = 1;

                message.mtype = 1 + i;                    //because of enumeration from 1
                strcpy(message.message, temp_buffer); //copy the command.
                message.index = client_index;              //for server response.

                temp_buffer = strtok(NULL, "\n");
                if (temp_buffer != NULL) {
                    strcpy(message.message, temp_buffer); //validation of args in server!
                }
                if (temp_buffer == NULL && message.mtype != TIME && message.mtype != END) {
                    valid = 0;      //two step validation
                }
                if (temp_buffer != NULL && (message.mtype == TIME || message.mtype == END)) {
                    valid = 0;
                }
            }
        }

        if (valid == 0) {
            fprintf(stderr, "Line %s contains invalid command!\n", buffer);
            continue;
        }
        send_message(server_queue, &message);
        printf("Sent msg %s of opcode %li\n", message.message, message.mtype);
        sent++;
    }

    while (sent != 0) {//send off all then wait for responses.

        struct msgbuffer_new response;
        receive_message(client_queue, &response);

        if (response.mtype == END) {
            printf("Mother program has come to an end, terminating now.\n");
            mother = 0;
            exit(EXIT_SUCCESS);
        }

        printf("Got response for an op: %s, result: %s\n", valid_strings[response.mtype - 1], response.message);
        sent--;
    }
    return 0;
}
