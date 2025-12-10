#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
typedef struct page {
int page_id;
int reference_bit;
struct page *next;
// @other auxiliary
} Node;
// @create an active list
// @create an inactive list
void *player_thread_func() {
// @Add code for player
pthread_exit(0);
}
void *checker_thread_func() {
// @Add code for checker
pthread_exit(0);
}
int main(int argc, char *argv[])
{
int n = atoi(argv[1]);
int m = atoi(argv[2]);
// @create a random reference string of N
/* Create two workers */
pthread_t player;
pthread_t checker;
pthread_create(&player, NULL, player_thread_func, NULL);
pthread_create(&checker, NULL, checker_thread_func, NULL);
pthread_join(player, NULL);
pthread_join(checker, NULL);
printf("Page_Id, Total_Referenced\n");
//@Print out the statistics of page references
printf("Pages in active list: ");
//@Print out the list of pages in active list
printf("Pages in active list: ");
//@Print out the list of pages in inactive list
/*free up resources properly */
}