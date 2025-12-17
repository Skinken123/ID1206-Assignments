#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>

#define FILE_SIZE 1024*1024 // 1 MB
#define SEM_PARENT_WRITTEN "/sem_parent_written"
#define SEM_CHILD_WRITTEN  "/sem_child_written"

int main() {
    int fd = open("file_to_map.txt", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    // Remove old semaphores
    sem_unlink(SEM_PARENT_WRITTEN);
    sem_unlink(SEM_CHILD_WRITTEN);

    // Create semaphores
    sem_t *sem_parent = sem_open(SEM_PARENT_WRITTEN, O_CREAT, 0644, 0);
    sem_t *sem_child  = sem_open(SEM_CHILD_WRITTEN,  O_CREAT, 0644, 0);

    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    char *mmap_ptr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    printf("Memory mapped at address: %p\n", mmap_ptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child
        char *text_to_write = "01234";
        char text_to_read[6];

        // Write to mmap
        memcpy(mmap_ptr, text_to_write, 5);
        msync(mmap_ptr, 5, MS_SYNC);

        // Signal child has written
        sem_post(sem_child);

        // Wait for parent to write
        sem_wait(sem_parent);

        // Read parent's write
        memcpy(text_to_read, mmap_ptr + 4096, 5);
        text_to_read[5] = '\0';
        printf("Child reads from mmap_ptr[4096]: %s\n", text_to_read);

        exit(0);
    } else { // Parent
        char *text_to_write = "56789";
        char text_to_read[6];

        // Wait for child to write first
        sem_wait(sem_child);

        // Write to mmap
        memcpy(mmap_ptr + 4096, text_to_write, 5);
        msync(mmap_ptr + 4096, 5, MS_SYNC);

        // Signal parent has written
        sem_post(sem_parent);

        // Read child's write
        memcpy(text_to_read, mmap_ptr, 5);
        text_to_read[5] = '\0';
        printf("Parent reads from mmap_ptr[0]: %s\n", text_to_read);

        wait(NULL); // wait for child to finish
    }

    // Cleanup
    munmap(mmap_ptr, FILE_SIZE);
    close(fd);
    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(SEM_PARENT_WRITTEN);
    sem_unlink(SEM_CHILD_WRITTEN);

    return 0;
}
