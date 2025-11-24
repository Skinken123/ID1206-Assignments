#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int num_threads = 0;
void *thread_func(void *); /* the thread function */

struct thread_args {
    int id;
    double *array;
    int start;
    int end;
};

int main(int argc, char *argv[]) {
    /* Seed the RNG, get num_threads and declare time variables */
    srand(time(0));
    struct timeval start, end;

    int num_threads = atoi(argv[1]);
    int array_length = atoi(argv[2]);

    /* Initialize an array of random values */
    double numbers[array_length];
    for (int i = 0; i < array_length; i++) {
        numbers[i] = rand() / (double)RAND_MAX;
    }

    /* Perform Serial Sum */
    double sum_serial = 0.0f;
    double time_serial = 0.0;
    gettimeofday(&start, NULL);
    for (int i = 0; i < array_length; i++) {
        sum_serial += numbers[i];
    }
    gettimeofday(&end, NULL);
    time_serial = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Serial Sum = %f, time = %f \n\n", sum_serial, time_serial);

    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = malloc(num_threads * sizeof(pthread_t));
    struct thread_args *args = malloc(num_threads * sizeof(struct thread_args));
    double time_parallel = 0.0;
    double sum_parallel = 0.0f;
    int workload = array_length/num_threads;

    gettimeofday(&start, NULL);
    for (int j = 0; j < num_threads; j++) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        args[j].id = j;
        args[j].array = numbers;
        args[j].start = j * workload;
        if (j + 1 == num_threads) args[j].end = array_length;
        else args[j].end = (j + 1) * workload;
        pthread_create(&workers[j], &attr, thread_func, &args[j]);
        pthread_attr_destroy(&attr);
    }

    /* Collect results from worker threads */
    for (int  j= 0; j < num_threads; j++) {
        void *result;
        pthread_join(workers[j], &result);
        sum_parallel += *(double *)result;
        free(result);
    }
    gettimeofday(&end, NULL);
    time_parallel = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Parallel Sum = %f, time = %f \n\n", sum_parallel, time_parallel);

    /* free up resources properly */
    free(workers);
    free(args);

    return 0;
}

void *thread_func(void *arg) {
    /* Assign each thread an id so that they are unique in range [0, num_threads -1] */
    struct thread_args *args = arg;
    int my_id = args->id;
    double *arr = args->array;
    int start = args->start;
    int end = args->end;

    // Sum array part
    double *my_sum = malloc(sizeof(double));
    *my_sum = 0;
    for (int i = start; i < end; i++) {
        *my_sum += arr[i];
    }
    // printf("Thread %d sum = %f\n", my_id, *my_sum);
    return my_sum;
}
