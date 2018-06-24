#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


#include <mqueue.h>
//define the structure for message
struct msgbuffer_define {
    long msg_type;      //we have to define this, it's type of the message (so we can later require certain type of msgs in msgrcv)
    char msg_content[256];
};
//this structure cannot exceed size of 4096 bytes

int main() {

    //SysV queue mechanisms
    // 1)We get the key.
    key_t queue_creation_key = ftok(getenv("HOME"), 's');
    int queue_key;
    // 2)We invoke msgget on the key with those flags
    //TODO -> CHECK HOW TO OPEN ON OTHER SIDE BOTH QUEUES
    if ((queue_key = msgget(queue_creation_key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR )) < 0)
        perror("MSGget failed. cannot get sysv queue");
    // 3)We can send sum msgs
    // EXAMPLE FLAGS ->
    // IPC_NOWAIT -> dont wait if full or empty,
    // MSG_EXCEPT -> DIFFERENT THAN TYPE,
    // MSG_NOERROR -> TRUNCATE WHEN TOO BIG MSG
    struct msgbuffer_define my_message;
    struct msgbuffer_define rc_message;
    my_message.msg_type = 1;
    strcpy(my_message.msg_content, "This is my message of type 1");

    if (msgsnd(queue_key, &my_message, sizeof(my_message) - sizeof(long),0)<0)
        perror("Error msgsnd");
    if (msgrcv(queue_key, &rc_message, sizeof(rc_message)-sizeof(long),0,MSG_NOERROR)<0)
        perror("Error msgrcv");

    printf("Read message : %s\n",rc_message.msg_content);

    if(msgctl(queue_key,IPC_RMID,NULL)<0)
        perror("Error deletequeue");


    //POSIX type msg-queues
    char buffer[]="This is typical posix message";
    char buffer_rcv[256];
    // 1) Set attrs
    struct mq_attr queue_attributes;
    queue_attributes.mq_maxmsg=10;
    queue_attributes.mq_msgsize=sizeof(buffer);
    mqd_t queue;
    // 2) Open queue, mode for creating rights oflag the same + someother sutff (O)NONBLOCK
    if((queue=mq_open("/posix_queue_1",O_CREAT | O_EXCL | O_RDWR, S_IRWXU , &queue_attributes ))==(mqd_t)-1)
        perror("Error POSIX queue create");
    // 3) Send msg
    if(mq_send(queue,buffer,sizeof(buffer),0)<0)
        perror("Error POSIX queue send");
    // 4)Receive -> you can receive priority
    if(mq_receive(queue,buffer_rcv,sizeof(buffer)+1,NULL)<0)
        perror("Error POSIX queue receive");

    printf("This is the message: %s\n",buffer_rcv);

    if(mq_close(queue)<0)
        perror("Error POSIX queue close");
    if(mq_unlink("/posix_queue_1")<0)
        perror("Error POSIX queue delete");

    //TODO -> NOTIFICATIONS SYSTEM
    return 0;
}