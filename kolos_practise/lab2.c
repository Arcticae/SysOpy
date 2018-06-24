#define _XOPEN_SOURCE 500
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>
int main() {
    //system functions IMPORTANT -> MAN OPEN FOR SYSV FLAGS AND MODE
    int file_descriptor = open("../example_filename", O_RDONLY);
    int new_file_descriptor = open("./example_filename2", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU );
    //reading the file 10*4 bytes per portion(charsize)
    //and writing the chars to the second file
    char buffer[256];
    int read_bytes, i;
    while ((read_bytes = read(file_descriptor, buffer, 10)) > 0) {
        for (i = 0; i < read_bytes; i++) {
            printf("%c", buffer[i]);
        }
        write(new_file_descriptor, buffer, read_bytes);
    }



    //close file descriptors

    close(file_descriptor);
    close(new_file_descriptor);

    //how to read last 8 bytes of the file? kolos quest?

    if ((file_descriptor = open("./example_filename", O_RDONLY)) < 0) {
        perror("Opening the file failed");
    }
    if (lseek(file_descriptor, -8, SEEK_END) < 0) {
        perror("Lseek failed");
    }

    if ((read_bytes = read(file_descriptor, buffer, 8)) < 0) {
        perror("Error reading the file");
        printf("Read bytes: %d\n",read_bytes);
    }

    printf("\nLast %d bytes: \n%s\n", read_bytes, buffer);

    //LIB functions     -> man fopen

    FILE *file_desc = fopen("./example_filename", "r");    // "r" "w" "a"
    FILE *second_file_desc = fopen("./example_filename2", "w+");
    while ((read_bytes = fread(buffer, sizeof(char), 10, file_desc)) > 0) {
        for (i = 0; i < read_bytes; i++)
            printf("%c", buffer[i]);
        fwrite(buffer, sizeof(char), read_bytes, second_file_desc);
    }

    if(fseek(second_file_desc,-8,SEEK_END)!=0)
        perror("");
    if((read_bytes=fread(buffer,sizeof(char),8,second_file_desc))!=8)
        perror("Fread mendo");
    printf("\n");
    for(i=0;i<read_bytes;i++)
        printf("%c",buffer[i]);

    fclose(file_desc);
    fclose(second_file_desc);


    //DIR handling
    printf("\n---------------------DIR HANDLING---------------------\n");

    DIR*directory;
    struct dirent *directory_entry;
    struct stat *entry_statistics;

    directory=opendir("./");        //open current catalogue

    while((directory_entry=readdir(directory))!=NULL){
        printf("\nEntry: %s" ,directory_entry->d_name);
    }
    rewinddir(directory);
    directory_entry=readdir(directory);
    printf("\n\nFirst file again: %s",directory_entry->d_name);
    stat(directory_entry->d_name,entry_statistics);

    //printf("\nStat dump: Mode: %d\n",(int)entry_statistics->st_mode);
    lstat("/.bashrc",entry_statistics);         //this is for link statistics

    if(mkdir("./somedir",S_IRWXU)<0)
        perror("Creating sumdir faild");
    if(rmdir("./somedir")<0)
        perror("Rmoving sumdir faild");
    if(chdir("/home/timelock")<0)
        perror("chdir not SUCCCCC");
    getcwd(buffer,256);
    printf("\nCWD: %s",buffer);

    //NFTW like

    if(nftw("~/Dokumenty/SysOpy/kolos_practise",))

    closedir(directory);
}