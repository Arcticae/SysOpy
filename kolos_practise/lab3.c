#include <stdio.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(){
    //getrlimit/setrlmit

    struct rlimit limits={
            .rlim_cur=10,
            .rlim_max=10
    };
    printf("CPU time limit: %d\n",limits.rlim_max);
    //this is fucking cool-aid-shit
    setrlimit(RLIMIT_CPU,&limits);
    getrlimit(RLIMIT_CPU,&limits);
    //once more
    printf("CPU time limit: (afterset) %d\n",limits.rlim_cur);
    //getppid -> obvious usage
    printf("MY pid is:%d\nMY parents pid is: %d\n",getpid(),getppid());

    int status=0;
    // man 3 execvp
    if(fork()==0){
        printf("Child process\n");
        exit(0);
    }
    wait(&status);
    printf("Exec status:%d",status);
    return 0;
}