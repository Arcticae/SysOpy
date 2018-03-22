//
// Created by timelock on 21.03.18.
#define __USE_XOPEN
#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#define NFTW_MODE 11
#define MANUAL_MODE 22

#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ftw.h>
#
const char format[] = "%Y-%m-%d %H:%M:%S";

char*tmp_operand;
time_t tmp_time;
time_t compare_days(time_t mod_time,time_t given_date){
    return (time_t) mod_time-given_date;
/*
    struct tm*modtime=localtime(&mod_time);
    struct tm*giventime=localtime(&given_date);
    struct tm*resulttime=malloc(sizeof(struct tm));





    resulttime->tm_year=modtime->tm_year-giventime->tm_year;
    resulttime->tm_mon=modtime->tm_mon-giventime->tm_mon;
    resulttime->tm_mday=modtime->tm_mday-giventime->tm_mday;

    char temptime[20];
    strftime(temptime,20 , format, modtime);
    printf("%s\n",temptime);

    strftime(temptime,20 , format, giventime);
    printf("%s\n",temptime);

    strftime(temptime,20 , format, resulttime);
    printf("%s\n",temptime);

    if(resulttime->tm_year==0 && resulttime->tm_mon==0 && resulttime->tm_mday==0){
        return 0;
    }
    if(resulttime->tm_year>0 || resulttime->tm_year==0)
    {
        if(resulttime->tm_mon>0 || resulttime->tm_mon==0)
        {
            if(resulttime->tm_mday>0)return 1;
        }
    }
    free(resulttime);
    return -1; */
}
void show_file(const char*file_path,const struct stat*file_stats){
    if(S_ISREG(file_stats->st_mode))
    {
        printf(" %lld\t", file_stats->st_size);//todo
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

            stat(element_path,&file_stats);

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
                    manual_display(element_path,operand,given_time);
                }

            }


            dir_entry=readdir(current_dir);


        }

    closedir(current_dir);



}

int nftw_show(const char*path,const struct stat* file_stats,int typeflag,struct FTW*ftwb){

    if(typeflag==FTW_F) {

        if (strcmp(tmp_operand, "=") == 0 && compare_days(file_stats->st_mtim.tv_sec, tmp_time) == 0) {
            show_file(path, file_stats);
        }
        if (strcmp(tmp_operand, ">") == 0 && compare_days(file_stats->st_mtim.tv_sec, tmp_time) > 0) {
            show_file(path, file_stats);
        }
        if (strcmp(tmp_operand, "<") == 0 && compare_days(file_stats->st_mtim.tv_sec, tmp_time) < 0) {
            show_file(path, file_stats);
        }
    }
    return 0;
}
void view_choice(const char*path,char*operand,char*given_time,int mode){

    char*real_path=realpath(path,NULL);

    struct tm*time_struct=malloc(sizeof(struct tm));
    strptime(given_time,format,time_struct);
    time_t tmp_epoch_time=mktime(time_struct);



    if(mode==NFTW_MODE){
        tmp_operand=operand;
        tmp_time=tmp_epoch_time;    //assigning the globals

        nftw(real_path,nftw_show,10,FTW_PHYS);

    }else if(mode==MANUAL_MODE){
        manual_display(real_path,operand,tmp_epoch_time);
    }

    free(real_path);
    free(time_struct);
}

int main(int argc,char**argv){



    //view_choice("../","<","2018-03-21 16:34:23",MANUAL_MODE);//example use
    printf("\n\n\n");
    view_choice("../","<","2018-03-21 16:34:23",NFTW_MODE);//example use
    return 0;
}