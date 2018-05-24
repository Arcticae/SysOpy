#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <wait.h>

#define MAX_LINE 256
#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096
#define MAX_FILTER 1024
int thread_amount;
int PGM_width;
int PGM_height;
int PGM_max;
int filter_size;

double filter[MAX_WIDTH][MAX_HEIGHT];
int PGM[MAX_WIDTH][MAX_HEIGHT];
int PGM_out[MAX_WIDTH][MAX_HEIGHT];

struct thread_wrapper {
    pthread_t thread_id;
    int row;
};

void failure(const char *communicate) {
    perror(communicate);
    exit(EXIT_FAILURE);
}

void parse_file(char *input_path) {
    char buffer[MAX_LINE];
    FILE *input = fopen(input_path, "r");

    if (input == NULL)
        failure("Opening of file went wrong:");


    fscanf(input, "%s", buffer);  //1st line
    if (strcmp("P2", buffer) != 0)
        failure("This is not a PGM");

    fscanf(input, "%s", buffer);
    PGM_width = (int) strtol(buffer, NULL, 10);

    fscanf(input, "%s", buffer);
    PGM_height = (int) strtol(buffer, NULL, 10);

    fscanf(input, "%s", buffer);
    PGM_max = (int) strtol(buffer, NULL, 10);

    if (PGM_width < 0 || PGM_width > MAX_WIDTH || PGM_height < 0 || PGM_height > MAX_HEIGHT || PGM_max < 0)
        failure("Not correct format of image");

    printf("Width: %d\nHeight: %d\nPGM_max: %d\n", PGM_width, PGM_height, PGM_max);

    int i, j;
    for (i = 0; i < PGM_height; i++)
        for (j = 0; j < PGM_width; j++) {
            fscanf(input, "%s", buffer);
            PGM[i][j] = (int) strtol(buffer, NULL, 10);
            if (PGM[i][j] > 255 || PGM[i][j] < 0)
                failure("Not correct format of one pxl:");
        }

    fclose(input);

}

void parse_filter(char *path) {
    char buffer[MAX_LINE];
    FILE *filter_file = fopen(path, "r");

    if (filter_file == NULL)
        failure("File opening went wrong: ");

    fscanf(filter_file, "%s", buffer);
    filter_size = (int) strtol(buffer, NULL, 10);
    if (filter_size < 1.0 || filter_size > MAX_FILTER)
        failure("Not correct filter size");
    int i, j;

    for (i = 0; i < filter_size; i++)
        for (j = 0; j < filter_size; j++) {
            fscanf(filter_file, "%s", buffer);
            filter[i][j] = strtod(buffer, NULL);
            if (filter[i][j] > 1.0 || filter[i][j] < 0.0)
                failure("Not correct format of one pxl:");
        }
    printf("Filter size: %d\n", filter_size);
    fclose(filter_file);
}

void process_pixel(int *pxl, int x, int y) {
    double filtered = 0.0;
    int tmp;
    int i, j;
    for (i = 0; i < filter_size; i++)
        for (j = 0; j < filter_size; j++) {
            filtered += (PGM[1 > (tmp = (int) (x - ceil((double) filter_size / 2.0) + i)) ? 1 : tmp]
            [1 > (tmp = (int) (y - ceil((double) filter_size / 2.0) + j)) ? 1 : tmp]
                        ) * filter[i][j];
        }
    filtered = round(filtered);
    *pxl = (int) filtered;

}

void *thread_task(void *args) {
    struct thread_wrapper *info = args;
    int row = info->row;
    int i;
    while (row < PGM_height) {
        for (i = 0; i < PGM_width; i++) {
            process_pixel(&PGM_out[row][i], row, i);
        }
        row += thread_amount;
    }
    return 0;
}

void save_progress(FILE *out) {
    int i, j;
    fprintf(out, "P2\n");
    fprintf(out, "%d %d\n", PGM_width, PGM_height);
    fprintf(out, "%d\n", PGM_max);
    for (i = 0; i < PGM_width; i++) {
        for (j = 0; j < PGM_height; j++) {
            fprintf(out, "%d ", PGM_out[i][j]);
        }
        fprintf(out, "\n");
    }
}

void measure_times(struct timeval *start_time) {
    struct timeval stop_time;
    gettimeofday(&stop_time, NULL);
    FILE *results = fopen("./measurements", "a");
    timersub(&stop_time, start_time, &stop_time);
    fprintf(results,
            "Test Results: \nTime of execution: %ld.%ldsec\nThread number: %d \nFilter size: %d\nImage resolution: %dx%d\n\n",
            stop_time.tv_sec, stop_time.tv_usec, thread_amount, filter_size, PGM_width, PGM_height);
    fclose(results);

}

int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Give me enough args: thread amount, input file path, filter path, filter size and  output path.\n");
    }

    if (fork() == 0) {
        execl(realpath("./filter_generator", NULL), "filter_generator", "./filter", argv[4], NULL);
        failure("Executing not succ");
    }
    int result_gen = 0;
    wait(&result_gen);

    thread_amount = (int) strtol(argv[1], NULL, 10);
    int i;
    parse_file(argv[2]);
    parse_filter(argv[3]);
    FILE *output = fopen(argv[5], "w+");

    struct thread_wrapper threads_inf[thread_amount];
    struct timeval start_time;

    gettimeofday(&start_time, NULL);
    for (i = 0; i < thread_amount; i++) {
        threads_inf[i].row = i;
        pthread_create(&threads_inf[i].thread_id, NULL, thread_task, (void *) &threads_inf[i]);
    }

    for (i = 0; i < thread_amount; i++) {
        pthread_join(threads_inf[i].thread_id, NULL);

    }
    measure_times(&start_time);

    save_progress(output);
    fclose(output);
    printf("Success! Check output.");
    return 0;
}