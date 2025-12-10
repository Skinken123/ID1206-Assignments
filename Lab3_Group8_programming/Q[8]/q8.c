#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct page {
    int page_id;
    int reference_bit;
    int ref_count;          
    struct page *next;
} Node;

Node *active = NULL;
Node *inactive = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int n, m;
int done = 0;
int ref_string[1000];
Node **pages;

// push node at end of list
void push_tail(Node **head, Node *p) {
    p->next = NULL;
    if (!*head) { 
        *head = p; return; 
    }
    Node *c = *head;
    while (c->next) c = c->next;
    c->next = p;
}

// remove node from list
void remove_node(Node **head, Node *p) {
    Node *c = *head, *prev = NULL;
    while (c) {
        if (c == p) {
            if (prev) prev->next = c->next;
            else *head = c->next;
            return;
        }
        prev = c;
        c = c->next;
    }
}

void *player_thread_func() {
    for (int i = 0; i < 1000; i++) {
        int pid = ref_string[i];
        Node *p = pages[pid];

        pthread_mutex_lock(&lock);

        p->reference_bit = 1;               // mark referenced
        remove_node(&inactive, p);
        remove_node(&active, p);
        push_tail(&active, p);              // move to active tail

        // if active > 70% n, move 20% to inactive
        int active_count = 0;
        for (Node *c = active; c; c = c->next) active_count++;

        if (active_count > 0.7 * n) {
            int move_k = (int)(0.2 * n);
            for (int k = 0; k < move_k && active; k++) {
                Node *front = active;
                active = active->next;
                push_tail(&inactive, front);
            }
        }

        pthread_mutex_unlock(&lock);
        usleep(10);
    }

    pthread_mutex_lock(&lock);
    done = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    pthread_exit(0);
}

void *checker_thread_func() {
    pthread_mutex_lock(&lock);
    while (!done) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += m * 1000;
        pthread_cond_timedwait(&cond, &lock, &ts);

        // scan active list
        for (Node *c = active; c; c = c->next) {
            if (c->reference_bit) {
                c->ref_count++;
                c->reference_bit = 0;
            }
        }
    }
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    // Seed the RNG
    srand(time(0));

    if (argc < 3) { 
        printf("Usage: ./a.out n m\n"); return 1; 
    }

    n = atoi(argv[1]);
    m = atoi(argv[2]);

    // create n pages, all pages are added to inactive list from the start since none of them are in use at program start
    pages = malloc(sizeof(Node*) * n);
    for (int i = 0; i < n; i++) {
        pages[i] = calloc(1, sizeof(Node));
        pages[i]->page_id = i;
        push_tail(&inactive, pages[i]);
    }

    // generate reference string of 1000
    for (int i = 0; i < 1000; i++)
        ref_string[i] = rand() % n;

    /* Create two workers */
    pthread_t player;
    pthread_t checker;
    pthread_create(&player, NULL, player_thread_func, NULL);
    pthread_create(&checker, NULL, checker_thread_func, NULL);

    pthread_join(player, NULL);
    pthread_join(checker, NULL);

    int total_refs = 0;
    printf("Page_Id, Total_Referenced\n");
    for (int i = 0; i < n; i++) {
        printf("%d, %d\n", i, pages[i]->ref_count);
        total_refs += pages[i]->ref_count;
    }
    printf("Total counted refrences: %d\n", total_refs);

    printf("Pages in active list: ");
    for (Node *c = active; c; c = c->next) {
        printf("%d ", c->page_id);
    } 
    printf("\n");

    printf("Pages in inactive list: ");
    for (Node *c = inactive; c; c = c->next) {
        printf("%d ", c->page_id);
    } 
    printf("\n");

    // free memory
    for (int i = 0; i < n; i++) {
        free(pages[i]);
    } 
    free(pages);

    return 0;
}