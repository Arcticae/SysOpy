#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main() {
    char buffer[256];
    char sent_string[] = "Hello frend!";
    int pipe_descriptors[2];
    pid_t childpid;
    int status;
    //simple read/write

    //1 is for wrtiting, 0 is for reading.

    /*pipe(pipe_descriptors);
    if((childpid=fork())==0){
        close(pipe_descriptors[1]);
        if(read(pipe_descriptors[0],buffer,256)<0)
            perror("Errno read");
        printf("Child: i haz read sum mesage :)))\n Mesag is : %s",buffer);
    }
    else{
        close(pipe_descriptors[0]);
        if(write(pipe_descriptors[1],sent_string,sizeof(sent_string))!=sizeof(sent_string))
            perror("Errno write");
    }

    waitpid(childpid,&status,0);
    printf("Child one has finished their job : Status : %d\n",status);
    */

    //next situation. ( write -> dup2(fd,stdin) -> execlp(prog,prog,args...,NULL) )
    pipe(pipe_descriptors);
    if ((childpid = fork()) == 0) {
        close(pipe_descriptors[1]);
        dup2(pipe_descriptors[0], STDIN_FILENO);
        execlp("grep", "grep", "frend", NULL);
        printf("I should not be here\n");
    } else {

        if (write(pipe_descriptors[1], sent_string, sizeof(sent_string)) != sizeof(sent_string))
            perror("Error write");
    }
    waitpid(childpid, &status, 0);

    //third option (POPEN) -> you can get the status by status=pclose(pipe)

    FILE *pipe_content;
    if ((pipe_content = popen("grep frend", "w")) == NULL)
        perror("Errno popen");
    if (fwrite(sent_string, sizeof(char), sizeof(sent_string), pipe_content) <= 0)
        perror("Errno fwrite popen");
    status = pclose(pipe_content);
    printf("Status of popen is: %d\n", status);

    //fourth option (MKFIFO)     S_ISFIFO/S_ISREG macros for checking files if fifo :)

    int fifo_filedescriptor;
    mkfifo("./fifko",S_IRWXU);

    if(fork()==0){
        fifo_filedescriptor=open("./fifko",O_RDONLY);
        int i;
        int readbytes=read(fifo_filedescriptor,buffer,256);
        for(i=0;i<readbytes;i++){
            printf("%c",buffer[i]);
        }

    }else{
        fifo_filedescriptor=open("./fifko",O_WRONLY);

        if(write(fifo_filedescriptor,sent_string,sizeof(sent_string))!=sizeof(sent_string))
            perror("error write");
    }

    return 0;
}