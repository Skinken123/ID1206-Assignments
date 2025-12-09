#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_pages>\n", argv[0]);
        return 0;
    }
    int num_pages = atoi(argv[1]);
    int page_size = getpagesize();
    int allocation_size = num_pages * page_size;
    printf("Number of pages %d, allocation size %d\n\n", num_pages, allocation_size);

    void *pointer = malloc(allocation_size);

    // Comment out if running without memset
    memset(pointer, 0, allocation_size);

    free(pointer);
    return 0;
}