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

    time_t start,end;
    int num_pages = atoi(argv[1]);
    int option = atoi(argv[2]);
    int page_size = getpagesize();
    int allocation_size = num_pages * page_size;

    printf("Allocating %d pages of %d bytes \n", num_pages, page_size);
    char *addr;
    // @Add the start of Timer here
    // Option 1: @Add mmap below to allocate num_pages anonymous pages
    if (option == 1) {
        addr = mmap(NULL, allocation_size);
    }
    // Option 2: @Add mmap below to allocate num_pages anonymous pages but using "huge" pages
    if (option == 2) {
        addr = (char*) mmap(NULL);
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

    // @Add the end of Timer here
    // @Add printout of elapsed time in cycles
    for(int i=0; (i<num_pages && i<16); i++){
        printf("%c ", addr[i*page_size]);
    }
    printf("\n");

    munmap(addr, page_size*num_pages);
}
