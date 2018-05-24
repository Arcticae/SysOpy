//
// Created by timelock on 24.05.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_FILTER 1024

void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {

    if (argc < 2)
        failure("Give me filepath and size of the filter in args");

    FILE *filter = fopen(argv[1], "w+");
    if (filter == NULL)
        failure("File creation went wrong:");

    int c = (int) strtol(argv[2], NULL, 10);
    if (c < 0 || c > MAX_FILTER)
        failure("Not correct size of filter");

    srand((unsigned) time(NULL));
    int i = 0,j=0, total_size = c * c;
    double *output = calloc((size_t) total_size, sizeof(double));
    double sum = 0.0, random;

    while (i<total_size) {
        random = drand48()/total_size*2;
        output[i++] += random;
        sum += random;
    }
    if(sum<1.0){
        int index=rand()%total_size;
        double diff=1.0-sum;
        output[index]+=diff;
        sum+=diff;
    }
    fprintf(filter,"%d\n",c);
    for(i=0;i<c;i++){
        for(j=0;j<c;j++)
            fprintf(filter,"%f ",output[i+ j*i]);
        fprintf(filter,"\n");
    }


    printf("\nsum: %f\n", sum);
}