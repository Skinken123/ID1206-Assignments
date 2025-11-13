#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

int main() {
    // Seed the RNG
    srand(time(0)); 

    // Create array of N size and populate with random numbers
    int N = 100;
    int numbers[N];
    for(int i = 0; i < N; i++) {
        numbers[i] = rand() % 2;
    }

    // Create child processes
    int pid = fork(); 
    int pid2;
    if (pid > 0) pid2 = fork(); //pid == 0 cild 1, pid2 == 0 child 2, parent pid2 > 0

    // Array summation
    if (pid == 0) {
        int sum1 = 0;
        for (int i = 0; i < N/2 - 1; i++) {
            sum1 += numbers[i];
        }
        printf("Sum 1 %d\n", sum1);
        exit(sum1);
    }
    if (pid2 == 0) {
        int sum2 = 0;
        for (int i = N/2 - 1; i < N; i++) {
            sum2 += numbers[i];
        }
        printf("Sum 2 %d\n", sum2);
        exit(sum2);
    }
    
    wait(NULL);
    wait(NULL);
    printf("Parent done\n");

    return 0;
}