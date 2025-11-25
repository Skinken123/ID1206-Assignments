#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h> // C11 Library for atomic operations

typedef struct arguments {
    int id;
    int opt;
} Arguments;

int num_threads = 0;
typedef struct node {
    int node_id; //a unique ID assigned to each node
    struct node *next;
} Node;

Node *top = NULL; // top of stack
pthread_mutex_t stack_mutex;
int node_id_counter = 0;

static _Atomic(Node*) atomic_top = NULL;   // atomic pointer to top of stack
static _Atomic(int) atomic_node_id_counter = 0;    // atomic unique ID generator

/*Option 1: Mutex Lock*/
void push_mutex(int idd) {
    Node *new_node;
    new_node = malloc(sizeof(Node));
    //update top of the stack below
    //assign a unique ID to the new node
    pthread_mutex_lock(&stack_mutex);
    /* assign unique ID while holding the lock to avoid races */
    new_node->node_id = node_id_counter++;
    new_node->next = top;
    top = new_node;
    
    //printf("%d ID is pushing a new node ID %d\n", idd, new_node->node_id);
    pthread_mutex_unlock(&stack_mutex);
}
int pop_mutex(int idd) {
    pthread_mutex_lock(&stack_mutex);

    if (top == NULL) {
        pthread_mutex_unlock(&stack_mutex);
        return -1; /* empty stack -> sentinel value */
    }

    Node *old_node = top;
    top = old_node->next;
    int id = old_node->node_id;

    //printf("%d ID is poping a old node ID %d\n", idd, id);
    pthread_mutex_unlock(&stack_mutex);

    free(old_node);
    return id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() {
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) return;

    new_node->node_id = atomic_fetch_add(&atomic_node_id_counter, 1);

    Node *old_top;
    do {
        old_top = atomic_load(&atomic_top);  // read current top
        new_node->next = old_top;     // point new node to current top
    } while (!atomic_compare_exchange_weak(&atomic_top, &old_top, new_node));
}
int pop_cas() {
    Node *old_top;
    Node *next;

    do {
        old_top = atomic_load(&atomic_top);  // read current top
        if (!old_top) return -1;      // empty stack
        next = old_top->next;         // node after top
    } while (!atomic_compare_exchange_weak(&atomic_top, &old_top, next));

    int id = old_top->node_id;
    free(old_top);
    return id;
}

/* the thread function */
void *thread_func(void *arg) {
    /* Assign each thread an id so that they are unique in range [0, num_thread -1] */
    Arguments *args = arg;
    int my_id = args->id;
    int opt = args->opt;

    if( opt==0 ) {
        push_mutex(my_id);
        push_mutex(my_id);
        pop_mutex(my_id);
        pop_mutex(my_id);
        push_mutex(my_id);
        printf("Mutex Thread %d: exit\n", my_id);
    } else {
        push_cas();
        push_cas();
        pop_cas();
        pop_cas();
        push_cas();
        printf("CAS Thread %d: exit\n", my_id);
    }
    pthread_exit(0);
}

void count_stack(Node *stack_top) {
    Node *current = stack_top;
    int count = 0;
    while (current) {
        current = current->next;
        count++;
    }
    printf("Nodes left on the stack: %d\n\n", count);
}


int main(int argc, char *argv[]) {
    for (int j = 1; j < 7; j++) {
        num_threads = atoi(argv[j]);

        /* Option 1: Mutex */
        pthread_t *workers = malloc(num_threads * sizeof(pthread_t));;
        Arguments *args = malloc(num_threads * sizeof(Arguments));
        for (int i = 0; i < num_threads; i++) {
            args[i].id = i;
            args[i].opt = 0;

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&workers[i], &attr, thread_func, &args[i]);
            pthread_attr_destroy(&attr);
        }
        for (int i = 0; i < num_threads; i++) {
            pthread_join(workers[i], NULL);
        }
        //Print out all remaining nodes in Stack
        printf("Mutex: Remaining nodes \n");
        count_stack(top);
        /*free up resources properly */
        free(workers);
        free(args);
        /* Option 2: CAS */
        workers = malloc(num_threads * sizeof(pthread_t));;
        args = malloc(num_threads * sizeof(Arguments));
        for (int i = 0; i < num_threads; i++) {
            args[i].id = i;
            args[i].opt = 1;

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&workers[i], &attr, thread_func, &args[i]);
            pthread_attr_destroy(&attr);
        }
        for (int i = 0; i < num_threads; i++) {
            pthread_join(workers[i], NULL);
        }
        //Print out all remaining nodes in Stack
        printf("CAS: Remaining nodes \n");
        count_stack(atomic_load(&atomic_top));
        /*free up resources properly */
        free(workers);
        free(args);
        printf("Lap finished\n\n");
    }

    return 0;
}