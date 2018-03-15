//
// Created by timelock on 13.03.18.
//

#ifndef PROJECT_LIBRARY_H
#define PROJECT_LIBRARY_H

struct array_blocks{
    char** array;
    int block_number;
    int is_static;
};
extern char*static_array[1000000];

struct array_blocks* create_array(int blocks,int is_static);

void delete_array(struct array_blocks*array);

void delete_block_at(struct array_blocks*array,int index);

void add_block_at(struct array_blocks*array,int index, char*blk);

char* get_nearest_block_bysum(struct array_blocks*array,int blocknumber);



#endif //PROJECT_LIBRARY_H
