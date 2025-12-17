#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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

/* ===== Reader thread ===== */
void *reader_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    for (int i = args->start_index; i < args->end_index; i++) {
        int offset = args->list[i].offset;
        int bytes  = args->list[i].bytes;

        ssize_t r = pread(args->file_descriptor,
                          (char *)args->buffer + offset,
                          bytes,
                          offset);

        if (r != bytes) {
            perror("read error");
        }
    }

    free(args);
    return NULL;
}

/* ===== Writer thread ===== */
void *writer_thread_func(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    for (int i = args->start_index; i < args->end_index; i++) {
        int offset = args->list[i].offset;
        int bytes  = args->list[i].bytes;

        ssize_t w = pwrite(args->file_descriptor,
                           (char *)args->buffer + offset,
                           bytes,
                           offset);

        if (w != bytes) {
            perror("write error");
        }
    }

    free(args);
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <num_bytes> <num_threads>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int num_bytes   = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    const int LIST_SIZE = 100;
    const int SEQ_REQUEST_SIZE  = 16384;
    const int RAND_REQUEST_SIZE = 128;
    const int PAGE_SIZE = 4096;
    const int NUM_OFFSETS = num_bytes / PAGE_SIZE;
    const int THREAD_WORKLOAD = LIST_SIZE / num_threads;

    struct timespec start, end;
    pthread_t threads[num_threads];
    int file_descriptor;

    void *buffer = calloc(1, num_bytes);

    /* ===== Build request lists ===== */
    ListData list1[LIST_SIZE];
    ListData list2[LIST_SIZE];

    int taken_offsets[NUM_OFFSETS];
    memset(taken_offsets, 0, sizeof(taken_offsets));

    for (int i = 0; i < LIST_SIZE; i++) {
        /* Sequential */
        list1[i].offset = i * SEQ_REQUEST_SIZE;
        list1[i].bytes  = SEQ_REQUEST_SIZE;

        /* Random */
        int found = 0;
        while (!found) {
            int off = rand() % NUM_OFFSETS;
            if (!taken_offsets[off]) {
                taken_offsets[off] = 1;
                list2[i].offset = off * PAGE_SIZE;
                list2[i].bytes  = RAND_REQUEST_SIZE;
                found = 1;
            }
        }
    }

    /* ===== Arrays for simple loop ===== */
    ListData *lists[2] = { list1, list2 };
    int request_sizes[2] = { SEQ_REQUEST_SIZE, RAND_REQUEST_SIZE };
    const char *list_names[2] = {
        "List 1 (Sequential)",
        "List 2 (Random)"
    };

    /* ===== Benchmark loop ===== */
    for (int l = 0; l < 2; l++) {

        /* ---------- WRITE ---------- */
        file_descriptor = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        clock_gettime(CLOCK_MONOTONIC, &start);

        int index = 0;
        for (int i = 0; i < num_threads; i++) {
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->list = lists[l];
            args->start_index = index;
            args->end_index = index + THREAD_WORKLOAD;
            args->buffer = buffer;
            args->file_descriptor = file_descriptor;

            pthread_create(&threads[i], NULL, writer_thread_func, args);
            index += THREAD_WORKLOAD;
        }

        for (int i = 0; i < num_threads; i++)
            pthread_join(threads[i], NULL);

        clock_gettime(CLOCK_MONOTONIC, &end);
        close(file_descriptor);

        double elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;

        double total_mb =
            (request_sizes[l] * LIST_SIZE) / (1024.0 * 1024.0);

        printf("Write %s: %.2f MB, %d threads, %.4f s, %.2f MB/s\n",
               list_names[l], total_mb, num_threads,
               elapsed, total_mb / elapsed);

        /* ---------- READ ---------- */
        file_descriptor = open("file.txt", O_RDONLY);
        clock_gettime(CLOCK_MONOTONIC, &start);

        index = 0;
        for (int i = 0; i < num_threads; i++) {
            ThreadArgs *args = malloc(sizeof(ThreadArgs));
            args->list = lists[l];
            args->start_index = index;
            args->end_index = index + THREAD_WORKLOAD;
            args->buffer = buffer;
            args->file_descriptor = file_descriptor;

            pthread_create(&threads[i], NULL, reader_thread_func, args);
            index += THREAD_WORKLOAD;
        }

        for (int i = 0; i < num_threads; i++)
            pthread_join(threads[i], NULL);

        clock_gettime(CLOCK_MONOTONIC, &end);
        close(file_descriptor);

        elapsed =
            (end.tv_sec - start.tv_sec) +
            (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Read  %s: %.2f MB, %d threads, %.4f s, %.2f MB/s\n\n",
               list_names[l], total_mb, num_threads,
               elapsed, total_mb / elapsed);
    }

    free(buffer);
    return 0;
}
