//
// Created by timelock on 21.03.18.
//
#define __USE_XOPEN
#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#define NFTW_MODE 11
#define MANUAL_MODE 22


#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char format[] = "%Y-%m-%d %H:%M:%S";

time_t compare_days(time_t mod_time,time_t given_date){
    return (time_t) mod_time-given_date;

}

void show_file(const char*file_path,const struct stat*file_stats){
    if(S_ISREG(file_stats->st_mode))
    {
        printf(" %ld\t", file_stats->st_size);
        printf((file_stats->st_mode & S_IRUSR) ? "r" : "-");
        printf((file_stats->st_mode & S_IWUSR) ? "w" : "-");
        printf((file_stats->st_mode & S_IXUSR) ? "x" : "-");
        printf((file_stats->st_mode & S_IRGRP) ? "r" : "-");
        printf((file_stats->st_mode & S_IWGRP) ? "w" : "-");
        printf((file_stats->st_mode & S_IXGRP) ? "x" : "-");
        printf((file_stats->st_mode & S_IROTH) ? "r" : "-");
        printf((file_stats->st_mode & S_IWOTH) ? "w" : "-");
        printf((file_stats->st_mode & S_IXOTH) ? "x" : "-");

        char temptime[20];
        strftime(temptime, 20, format, localtime(&file_stats->st_mtime));
        printf(" %s\t", temptime);
        printf(" %s\t", realpath(file_path, NULL));
        printf("\n");
    }

}
void manual_display(char*path,char*operand,time_t given_time){

    if(path==NULL){
        printf("The directory is NULL,give me the right directory\n");
        return;
    }

    DIR*current_dir=opendir(path);
    struct stat file_stats;
    char element_path[PATH_MAX];

    if(current_dir==NULL){
        printf("There was an error opening the wanted directory\n");
        return;
    }

    struct dirent *dir_entry=readdir(current_dir);

    while(dir_entry!=NULL){

        strcpy(element_path,path);
        strcat(element_path,"/");
        strcat(element_path,dir_entry->d_name);

        if(lstat(element_path,&file_stats)<0)exit(1);

        if(strcmp(dir_entry->d_name,".")!=0 && strcmp(dir_entry->d_name,"..")!=0 )
        {

            if (strcmp(operand, "=") == 0 && compare_days(file_stats.st_mtim.tv_sec, given_time) == 0) {
                show_file(element_path, &file_stats);
            }
            if (strcmp(operand, ">") == 0 && compare_days(file_stats.st_mtim.tv_sec, given_time) > 0) {
                show_file(element_path, &file_stats);
            }
            if (strcmp(operand, "<") == 0 && compare_days(file_stats.st_mtim.tv_sec, given_time) < 0) {
                show_file(element_path, &file_stats);
            }

            if(S_ISDIR(file_stats.st_mode)){
                pid_t  fork_pid=fork();
                if(fork_pid==0) {
                    manual_display(element_path, operand, given_time);
                    exit(EXIT_SUCCESS);

                }
                if(fork_pid<0)
                {
                    printf("Fork error\n");
                    return;
                }
            }

        }

    dir_entry=readdir(current_dir);



    }

    closedir(current_dir);



}

int main(int argc,char**argv){


    if(argc<4){
        printf("Wrong number of args, give me 5 like : path,operand,Year-Month-Day Hr:Min:Sec\n");
        return -1;
    }

    char*path=argv[1];
    char*given_time=argv[3];
    char*real_path=realpath(path,NULL);
    struct tm*time_struct=malloc(sizeof(struct tm));

    strptime(given_time,format,time_struct);
    time_t tmp_epoch_time=mktime(time_struct);

    manual_display(real_path,argv[2],tmp_epoch_time);


    free(real_path);
    free(time_struct);
    sleep(1);
    return 0;
}