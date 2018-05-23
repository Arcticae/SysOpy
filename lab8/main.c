#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 256
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096
#define MAX_FILTER 1024
int thread_amount;
int PGM_width;
int PGM_height;
int PGM_max;
int filter_size;
int i, j; //using that frequently

void failure(const char*communicate){
    perror(communicate);
    exit(EXIT_FAILURE);
}

int **parse_file(char *input_path) {
    char buffer[MAX_LINE];
    FILE *input = fopen(input_path, "r");

    if (input == NULL)
        failure("Opening of file went wrong:");


    fscanf(input, "%s", buffer);  //1st line

    fscanf(input, "%s", buffer);
    PGM_width = (int) strtol(buffer, NULL, 10);

    fscanf(input, "%s", buffer);
    PGM_height = (int) strtol(buffer, NULL, 10);

    fscanf(input, "%s", buffer);
    PGM_max = (int) strtol(buffer, NULL, 10);

    if (PGM_width < 0 || PGM_width > MAX_WIDTH || PGM_height < 0 || PGM_height > MAX_HEIGHT || PGM_max < 0)
        failure("Not correct format of image");

    printf("Width: %d\nHeight: %d\nPGM_max: %d\n", PGM_width, PGM_height, PGM_max);

    int **result = (int **) malloc(PGM_height * sizeof(int *));
    for (i = 0; i < PGM_height; i++)
        result[i] = malloc(PGM_width * sizeof(int));

    for (i = 0; i < PGM_height; i++)
        for (j = 0; j < PGM_width; j++) {
            fscanf(input, "%s", buffer);
            result[i][j] = (int) strtol(buffer, NULL, 10);
            if(result[i][j] > 255 || result[i][j] < 0)
              failure("Not correct format of one pxl:");
        }

    fclose(input);
    return result;

}

double** parse_filter(char*path){
    char buffer[MAX_LINE];
    FILE *filter_file = fopen(path, "r");

    if (filter_file == NULL)
        failure("File opening went wrong: ");

    fscanf(filter_file, "%s", buffer);
    filter_size = (int) strtol(buffer, NULL, 10);
    if(filter_size<0 || filter_size > MAX_FILTER)
        failure("Not correct filter size");

    double**filter =malloc(filter_size*sizeof(double*));
    for(i=0;i<filter_size;i++)
        filter[i]=malloc(sizeof(double));

    for(i=0;i<filter_size;i++)
        for(j=0;j<filter_size;j++){
            fscanf(filter_file, "%s", buffer);
            filter[i][j] = strtod(buffer,NULL);
            if(filter[i][j] > 1.0 || filter[i][j] < 0.0 )
                failure("Not correct format of one pxl:");
        }
}

int main(int argc, char **argv) {

    if (argc < 4) {
        printf("Give me enough args: thread amount, input file path, filter path, output path.\n");
    }
    thread_amount = (int) strtol(argv[1], NULL, 10);

    int **pgm = parse_file(argv[2]);
    double **filter=parse_filter(argv[2]);

    return 0;
}