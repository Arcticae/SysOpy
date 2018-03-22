#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#define LIB_MODE 11
#define SYS_MODE 22


int generate(char*dest_path,int record_amount,int record_length){
    FILE * file = fopen(dest_path,"w+");    // overwrites the existing file if exists
    FILE * random= fopen("/dev/urandom","r");
    if (file==NULL || random==NULL){
        printf("Error while generating the file, error number : %d (%s)\n", errno, strerror(errno));
        return -1;
    }
    char*buffer=malloc((record_length+1)*sizeof(char));

    int i;
    for(i=0;i<record_amount;i++){
        int j;

        if(fread(buffer,sizeof(char),(size_t) record_length,random)!=record_length){
            printf("There was a problem generating data\n");
            return 1;
        }



        for(j=0;j<record_length;j++){
            buffer[j]=(char) (abs(buffer[j])%25 +97);
        }
        buffer[record_length]=10;

            if(fwrite(buffer,sizeof(char),(size_t) record_length+1,file)!=record_length+1){
                printf("There was a problem while saving random records to file\nPlease,try again\n");
                return 1;
            }



    }
    fclose(file);
    fclose(random);
    free(buffer);

}

int library_sort(char*source_path,int record_amount,int record_length){

    char*record1=malloc((record_length+1)*sizeof(char));
    char*record2=malloc((record_length+1)*sizeof(char));
    long int offset=(long int) ((record_length+1)*sizeof(char)); //will be multiplied later by a factor
    int i,j;
    FILE *source=fopen(source_path,"r+");

    if(source==NULL){
        printf("No such file is available for sorting, please check your choice one more time\n");
        return 1;
    }


    for(i=0;i<record_amount;i++){
        fseek(source,offset*i,0);       //set cursor on source, offset*i from beginning of the file (whence arg=0)
        if(fread(record1,sizeof(char),(size_t) (record_length+1),source)!=record_length+1){
            printf("Error while reading from file\n");
            return -1;
        }

        for(j=0;j<i;j++){

            fseek(source,offset*j,0);

            if(fread(record2, sizeof(char),(size_t)(record_length+1),source)!=record_length+1){
                printf("Error while reading from file\n");
                return -1;
            }

            if(record2[0]>record1[0]){

                fseek(source,offset*j,0);       //fread moves the ptr so we have to fseek again

                if(fwrite(record1,sizeof(char),(size_t)record_length+1,source)!=record_length+1){
                    printf("Error while saving to file\n");
                    return -1;
                }

                fseek(source,offset*i,0);

                if(fwrite(record2,sizeof(char),(size_t)record_length+1,source)!=record_length+1){
                    printf("Error while saving to file\n");
                    return -1;
                }
                //THE RECORDS HAVE CHANGED THEIR LOCATIONS>>SWAP PTRS

                char*swp;
                swp=record2;
                record2=record1;
                record1=swp;

            }

        }

    }
    free(record1);
    free(record2);
    fclose(source);
    return 0;

}
int system_sort(char*source_path,int record_amount,int record_length){

    char*record1=malloc((record_length+1)*sizeof(char));
    char*record2=malloc((record_length+1)*sizeof(char));
    long int offset=(long int) ((record_length+1)*sizeof(char)); //will be multiplied later by a factor
    int i,j;

    int source=open(source_path,O_RDWR);        //gets the descriptor


    if(source<0){
        printf("An error occurred while opening the file, please check your choice one more time\n");
        return -1;
    }


    for(i=0;i<record_amount;i++){

        lseek(source,(off_t) offset*i,SEEK_SET);       //set cursor on source, offset*i from beginning of the file (whence arg=0)

        if(read(source,record1,(size_t)((record_length+1)*sizeof(char)))!=record_length+1){
            printf("Error while reading from file\n");
            return -1;
        }

        for(j=0;j<i;j++){

            lseek(source,(off_t) offset*j,SEEK_SET);

            if(read(source,record2,(size_t)((record_length+1)*sizeof(char)))!=record_length+1){
                printf("Error while reading from file\n");
                return -1;
            }

            if(record2[0]>record1[0]){

                lseek(source,(off_t) offset*j,SEEK_SET);       //fread moves the ptr so we have to fseek again

                if(write(source,record1,(size_t)(sizeof(char)*record_length+1))!=record_length+1){
                    printf("Error while saving to file\n");
                    return -1;
                }

                lseek(source,(off_t) offset*i,SEEK_SET);

                if(write(source,record2,(size_t)(sizeof(char)*record_length+1))!=record_length+1){
                    printf("Error while saving to file\n");
                    return -1;
                }
                //THE RECORDS HAVE CHANGED THEIR LOCATIONS>>SWAP PTRS

                char*swp;
                swp=record2;
                record2=record1;
                record1=swp;

            }

        }

    }
    free(record1);
    free(record2);
    close(source);
    return 0;

}
int lib_copy(char*source,char*dest,int record_amount,int record_length){

    FILE*from=fopen(source,"r");
    FILE*to=fopen(dest,"w+");

    if(from==NULL || to == NULL){
        printf("No such file is available for copying, please check your choice one more time\n");
        return -1;
    }

    char*record=malloc((record_length+1)*sizeof(char));

    int i;
    for(i=0;i<record_amount;i++){

        if(fread(record, sizeof(char),(size_t)(record_length+1),from)!=record_length+1){
            printf("Error while reading from file (copying)\n");
            return -1;
        }

        if(fwrite(record,sizeof(char),(size_t)record_length+1,to)!=record_length+1){
            printf("Error while saving to file (copying)\n");
            return -1;
        }


    }
    fclose(to);
    fclose(from);
    free(record);
    return 0;


}

int sys_copy(char*source,char*dest,int record_amount,int record_length){

    int from=open(source,O_RDONLY);
    int to=open(dest,O_WRONLY|O_TRUNC);

    if(from<0 || to <0){
        printf("No such file is available for copying, please check your choice one more time\n");
        return -1;
    }

    char*record=malloc((record_length+1)*sizeof(char));

    int i;
    for(i=0;i<record_amount;i++){

        if(read(from,record,(size_t)(record_length+1)*sizeof(char))!=record_length+1){
            printf("Error while reading from file (copying)\n");
            return -1;
        }

        if(write(to,record,(size_t)(record_length+1)*sizeof(char))!=record_length+1){
            printf("Error while saving to file (copying)\n");
            return -1;
        }


    }
    close(to);
    close(from);
    free(record);
}
void copy_join(char*source_raw,char*dest_raw,int record_amount,int record_length,int mode){

    char source[PATH_MAX];
    realpath(source_raw,source);

    char dest[PATH_MAX];
    realpath(dest_raw,dest);

    if(mode==SYS_MODE){
        if(sys_copy(source,dest,record_amount,record_length)<0){
            printf("There was an error while system-mode copying. Operation failed\n");
        } else return;
    }
    else if(mode==LIB_MODE){
        if(lib_copy(source,dest,record_amount,record_length)<0){
            printf("There was an error while library-mode copying. Operation failed\n");
        }else return;
    }
    else{
        printf("Wrong mode-type chosen. choose \"LIB_MODE\" or \"SYS_MODE\" as an argument to operation\n");
    }
}
void sort_join(char*source_raw,int record_amount,int record_length,int mode){

    char source[PATH_MAX];
    realpath(source_raw,source);


    if(mode==SYS_MODE){
        if(system_sort(source,record_amount,record_length)<0){
            printf("There was an error while system-mode copying. Operation failed\n");
        }
    }
    else if(mode==LIB_MODE){
        if(library_sort(source,record_amount,record_length)<0){
            printf("There was an error while library-mode copying. Operation failed\n");
        }
    }
    else{
        printf("Wrong mode-type chosen. choose \"LIB_MODE\" or \"SYS_MODE\" as an argument to operation\n");
    }
}

int main(int argc,char**argv){

    struct timeval* user_times=malloc(2*sizeof(struct timeval));
    struct timeval* sys_times=malloc(2*sizeof(struct timeval));
    struct timeval** real_times=malloc(2*sizeof(struct timeval*));

    for(int i=0;i<2;i++){

        real_times[i]=(struct timeval*)malloc(sizeof(struct timeval*));
    }


    struct rusage* before=(struct rusage*)malloc(sizeof(struct rusage));
    struct rusage* after=(struct rusage*)malloc(sizeof(struct rusage));




   if(argc<5){
        printf("There are no enough arguments. Give me at least 4\n");
        return -1;
    }

    if(strcmp(argv[1],"generate")==0){
        int record_amount=(int)strtol(argv[3],NULL,10);
        int record_length=(int)strtol(argv[4],NULL,10);

        gettimeofday(real_times[0],NULL);
        getrusage(RUSAGE_SELF,before);

        generate(argv[2],record_amount,record_length);

        gettimeofday(real_times[1],NULL);
        getrusage(RUSAGE_SELF,after);
    }
    else if(strcmp(argv[1],"sort")==0){

        if(argc<6){
            printf("There are no enough arguments. Give me at least 5 for sorting\n");
            return -1;
        }
        int record_amount=(int)strtol(argv[3],NULL,10);
        int record_length=(int)strtol(argv[4],NULL,10);
            if(strcmp(argv[5],"LIB_MODE")==0)
            {
                gettimeofday(real_times[0],NULL);
                getrusage(RUSAGE_SELF,before);

                sort_join(argv[2],record_amount,record_length,LIB_MODE);

                gettimeofday(real_times[1],NULL);
                getrusage(RUSAGE_SELF,after);

            }
            else if(strcmp(argv[5],"SYS_MODE")==0)
            {
                gettimeofday(real_times[0],NULL);
                getrusage(RUSAGE_SELF,before);

                sort_join(argv[2],record_amount,record_length,SYS_MODE);

                gettimeofday(real_times[1],NULL);
                getrusage(RUSAGE_SELF,after);
            }
            else {
                printf("Wrong mode-type chosen. choose \"LIB_MODE\" or \"SYS_MODE\" as an argument to operation\n");
                return -1;
            }
        }else if(strcmp(argv[1],"copy")==0){

        if(argc<7){
            printf("There are no enough arguments. Give me at least 6 for copying and pasting\n");
            return -1;
        }

        int record_amount=(int)strtol(argv[4],NULL,10);
        int record_length=(int)strtol(argv[5],NULL,10);

        if(strcmp(argv[6],"LIB_MODE")==0)
        {
            gettimeofday(real_times[0],NULL);
            getrusage(RUSAGE_SELF,before);

            copy_join(argv[2],argv[3],record_amount,record_length,LIB_MODE);

            gettimeofday(real_times[1],NULL);
            getrusage(RUSAGE_SELF,after);
        }
        else if(strcmp(argv[6],"SYS_MODE")==0)
        {
            gettimeofday(real_times[0],NULL);
            getrusage(RUSAGE_SELF,before);

            copy_join(argv[2],argv[3],record_amount,record_length,SYS_MODE);

            gettimeofday(real_times[1],NULL);
            getrusage(RUSAGE_SELF,after);
        }
        else {
            printf("Wrong mode-type chosen. choose \"LIB_MODE\" or \"SYS_MODE\" as an argument to operation\n");
            return -1;
        }
    }

    sys_times[0]=before->ru_stime;
    sys_times[1]=after->ru_stime;

    user_times[0]=before->ru_utime;
    user_times[1]=after->ru_utime;

    if(strcmp(argv[1],"copy")==0){
        printf("\nInvoking operation %s with arguments: \n%d of entries\nwhich are %d characters long",argv[1],(int)strtol(argv[4],NULL,10),(int)strtol(argv[5],NULL,10));
    }else{
        printf("\nInvoking operation %s with arguments: \n%d of entries\nwhich are %d characters long",argv[1],(int)strtol(argv[3],NULL,10),(int)strtol(argv[4],NULL,10));
    }

    printf("\nInvoking operation %s with arguments\n",argv[1]);
    printf("Realtime: %ld.%06lds\n",real_times[1]->tv_sec-real_times[0]->tv_sec,real_times[1]->tv_usec-real_times[0]->tv_usec);
    printf("System Time: %ld.%06lds\n",sys_times[1].tv_sec-sys_times[0].tv_sec,sys_times[1].tv_usec-sys_times[0].tv_usec);
    printf("User Time: %ld.%06lds\n\n",user_times[1].tv_sec-user_times[0].tv_sec,user_times[1].tv_usec-user_times[0].tv_usec);


    return 0;
}