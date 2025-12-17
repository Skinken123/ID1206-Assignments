#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

typedef struct {
    int offset;
    int bytes;
} ListData;

typedef struct {
    ListData *list;
    int start_index;
    int end_index;
    void *buffer;
    int file_descriptor;
} ThreadArgs;  

void *reader_thread_func(/*@input paramters*/) {
    // @Add code for reader threads
    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: read bytes_i from offset_i
    pthread_exit(0);
}
void *writer_thread_func(void *arg) {
    // @Add code for writer threads
    // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
    // @for each: write bytes_i to offset_i
    ThreadArgs *args = (ThreadArgs *)arg;

     ListData *list = args->list;
     int start_index = args->start_index;
     int end_index = args->end_index;
     char *buffer = (char *)args->buffer;  
     int file_descriptor = args->file_descriptor;

     for(int i = start_index; i < end_index; i++) {
          int offset = list[i].offset;
          int bytes = list[i].bytes;

          // writes sequentially to file
          ssize_t written_bytes = pwrite(file_descriptor, buffer + offset, bytes, offset);

          if (written_bytes != bytes) {
               perror("write error");
          }
     }

     pthread_exit(0);
}
int main(int argc, char *argv[]) {
    srand(time(0)); // Seed RNG
    int num_bytes = atoi(argv[1]); //number of bytes
    int num_threads = atoi(argv[2]); //number of threads
    const int LIST_SIZE = 100;
    const int SEQ_REQUEST_SIZE = 16384;
    const int RAND_REQUEST_SIZE = 128;
    const int PAGE_SIZE = 4096;
    const int NUM_OFFSETS = num_bytes / PAGE_SIZE;
    const int THREAD_WORKLOAD = LIST_SIZE / num_threads;
    // for timer
    struct timespec start,end;

    // @create a file for saving the data
    FILE *fileptr;
    int file_descriptor = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // @allocate a buffer and initialize it
    void *buffer = calloc(1, num_bytes);
    // @create two lists of 100 requests in the format of [offset, bytes]

    // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
    // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
    // @ensure no overlapping among these requests.

    // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
    // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
    // @ensure no overlapping among these requests.
    int taken_offsets[NUM_OFFSETS];
    memset(taken_offsets, 0, sizeof(taken_offsets));

    ListData list1[LIST_SIZE];
    ListData list2[LIST_SIZE];
    for (int i = 0; i < LIST_SIZE; i++) {
        list1[i] = (ListData){i * SEQ_REQUEST_SIZE, SEQ_REQUEST_SIZE};

        int found_offset = 0;
        while (!found_offset) {
            int offset = rand() % (NUM_OFFSETS);
            if (!taken_offsets[offset]) {
                taken_offsets[offset] = 1;
                list2[i] = (ListData){offset * PAGE_SIZE, RAND_REQUEST_SIZE};
                found_offset = 1;
            }
        }
    }

    // @start timing
    /* Create writer workers and pass in their portion of list1 */
    /* Wait for all writers to finish */
    // @close the file
    // @end timing

    //Start time
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_t threads[num_threads];
    int array_index = 0;
    for (int i = 0; i < num_threads; i++) {
        ThreadArgs *func_data = malloc(sizeof(ThreadArgs));
        func_data->list = list1;
        func_data->start_index = array_index;
        func_data->end_index = array_index + THREAD_WORKLOAD;
        func_data->buffer = buffer;
        func_data->file_descriptor = file_descriptor;
        pthread_create(&threads[i], NULL, writer_thread_func, func_data);
        array_index += THREAD_WORKLOAD;
    }

    for(int i = 0; i < num_threads; i++) {
          pthread_join(threads[i], NULL);
    }

    //End time
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    close(file_descriptor);
    double total_mb = (SEQ_REQUEST_SIZE * LIST_SIZE) / (1024.0 * 1024.0);
    double bandwidth = total_mb / elapsed;

    //@Print out the write bandwidth
    printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %fMB/s \n", total_mb, num_threads, elapsed, bandwidth);

    // @reopen the file
    // @start timing
    /* Create reader workers and pass in their portion of list1 */
    /* Wait for all reader to finish */
    // @close the file
    // @end timing

    //Start time
    clock_gettime(CLOCK_MONOTONIC, &start);

    

    //End time
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    //@Print out the read bandwidth
    //printf("Read %f MB, use %d threads, elapsed time %f s, write bandwidth: %fMB/s \n", 1, 2, 3 , 4);

    // @Repeat the write and read test now using List2
    /*free up resources properly */
    free(buffer);
}
