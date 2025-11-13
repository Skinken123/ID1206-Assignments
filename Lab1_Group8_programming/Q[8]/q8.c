#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

int main() {
    // Seed the RNG and declare time variables
    srand(time(0)); 
    struct timeval start, end;

    // Create array of N size and populate with random numbers
    int benchmark[] = {100, 1000, 10000, 100000, 1000000};
    int N;
    for (int i = 0; i < 5; i++) {
        N = benchmark[i];
        int numbers[N];
        for(int i = 0; i < N; i++) {
            numbers[i] = rand() % 2;
        }

        // Create pipes
        int pipe1[2], pipe2[2];
        if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
            perror("Pipe failed\n");
            exit(1);
        }

        // Create child processes and start time
        gettimeofday(&start, NULL);
        int pid = fork(); 
        int pid2;
        if (pid > 0) pid2 = fork(); //pid == 0 cild 1, pid2 == 0 child 2, parent pid2 > 0

        // Array summation
        if (pid == 0) {
            close(pipe1[0]);
            int sum1 = 0;
            for (int i = 0; i < N/2 - 1; i++) {
                sum1 += numbers[i];
            }
            write(pipe1[1], &sum1, sizeof(sum1));
            close(pipe1[1]);
            exit(0);
        }
        if (pid2 == 0) {
            close(pipe2[0]);
            int sum2 = 0;
            for (int i = N/2 - 1; i < N; i++) {
                sum2 += numbers[i];
            }
            write(pipe2[1], &sum2, sizeof(sum2));
            close(pipe2[1]);
            exit(0);
        }

        // Wait for child processes to finish
        wait(NULL);
        wait(NULL);
        
        // Get pipe data from child processes
        close(pipe1[1]);
        close(pipe2[1]);
        int sum1, sum2;
        read(pipe1[0], &sum1, sizeof(sum1));
        read(pipe2[0], &sum2, sizeof(sum2));
        close(pipe1[0]);
        close(pipe2[0]);

        // Print final sum and total time
        printf("The sum of the whole array is %d\n", sum1 + sum2);
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        printf("Total time %.8f\n", elapsed);
    }

    return 0;
}