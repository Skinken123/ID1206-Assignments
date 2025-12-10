#define _GNU_SOURCE 
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

// 2 MB
#define HUGE_PAGE_SIZE (2097152) 

int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Usage: ./%s <num_pages> <option 1/option 2>\n", argv[0]);
        return 0;
    }

    struct timespec start,end;

    int num_pages = atoi(argv[1]);
    int option = atoi(argv[2]);
    int page_size = getpagesize();
    int allocation_size = num_pages * page_size;

    printf("Allocating %d pages of %d bytes \n", num_pages, page_size);
    
    char *addr;

    //Start time
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (option == 1) {
        addr = (char*) mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }
    
    if (option == 2) {
        printf("Trying to allocate huge\n");
        addr = (char*) mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    }

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //the code below updates the pages
    char c = 'a';
    for(int i=0; i<num_pages; i++){
        addr[i*page_size] = c;
        c ++;
    }

    //End time
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %.9f seconds\n", elapsed);

    for(int i=0; (i<num_pages && i<16); i++){
        printf("%c ", addr[i*page_size]);

    }
    printf("\n");

    munmap(addr, page_size*num_pages);
}
