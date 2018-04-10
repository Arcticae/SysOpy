//
// Created by timelock on 28.03.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <errno.h>

#define max_length 100
#define max_argnum 20

int set_limits(char*cpu,char*mem){

    struct rlimit mem_limit;
    struct rlimit cpu_limit;

    cpu_limit.rlim_max=(rlim_t)strtol(cpu,NULL,10);
    cpu_limit.rlim_cur=(rlim_t)strtol(cpu,NULL,10);
    mem_limit.rlim_max=(rlim_t)strtol(mem,NULL,10)*1024*1024; //given in MiB
    mem_limit.rlim_cur=(rlim_t)strtol(mem,NULL,10)*1024*1024;
    if(setrlimit(RLIMIT_CPU,&cpu_limit)!=0) //cpu mem
    {
        printf("Could not set given cpu limits, reason:  %s\n",strerror(errno));
        return -1;
    }

    if(setrlimit(RLIMIT_AS,&mem_limit)!=0)
    {
        printf("Could not set given memory limits, reason:  %s\n",strerror(errno));
        return -1;
    }

    return 0;

}

int main(int argc, char**argv){

    if(argc<2){
        printf("You have given me not enough args, give me filepath\n");
        return -1;
    }

    FILE* batch_file=fopen(argv[1],"r");
    if(batch_file==NULL){
        printf("The batch file cannot be opened, please try again\n");
        return -1;
    }

    char*arguments[max_argnum];
    char line[max_length];
    int argnum;
    int status;
    struct rusage rusage_before,rusage_after;

    while(fgets(line,max_length,batch_file)){
        argnum=0;
        do{
            if(argnum==0){
                arguments[argnum]=strtok(line,"\t\n ");
                argnum++;
            }else {
                arguments[argnum]=strtok(NULL,"\t\n ");
                argnum++;
            }

        }while(arguments[argnum-1]!=NULL && argnum < max_argnum);

        if(argnum>=max_argnum){
            printf("The number of argumnets has exceeded the given limit. Please reedit the given file\n");
            return -1;
        }
        getrusage(RUSAGE_CHILDREN,&rusage_before);
        pid_t child_pid=fork();

        if(child_pid==0){
            if(set_limits(argv[2],argv[3])<0)return -1;
            execvp(arguments[0],arguments);
            exit(1);                       //the code below will not be executed if the process does exec properly
        }
        else if(child_pid<0){

            printf("Fork failed.\n");
            return -1;
        }

        wait(&status);
        getrusage(RUSAGE_CHILDREN,&rusage_after);

        if(WIFEXITED(status)==0){
            printf("The process: %s returned code %d, the task will now terminate.\n",arguments[0],status);
            return -1;
        }
        struct timeval user_result_time;
        struct timeval system_result_time;
        timersub(&rusage_after.ru_utime,&rusage_before.ru_utime,&user_result_time);
        timersub(&rusage_after.ru_stime,&rusage_before.ru_stime,&system_result_time);
        printf("SYS time of process: %s is : %d.%d\n",arguments[0],(int)system_result_time.tv_sec,(int)system_result_time.tv_usec);
        printf("USR time of process: %s is : %d.%d\n",arguments[0],(int)user_result_time.tv_sec,(int)user_result_time.tv_usec);

    }



    fclose(batch_file);
    return 0;
}