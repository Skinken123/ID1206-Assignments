#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

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
        addr = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }
    
    if (option == 2) {
        addr = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
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
    end = time(NULL);
    double elapsed = difftime(end, start);
    printf("Elapsed time: %.2f seconds\n", elapsed);

    for(int i=0; (i<num_pages && i<16); i++){
        printf("%c ", addr[i*page_size]);
    }
    printf("\n");

    munmap(addr, page_size*num_pages);
    return 0;
}
