#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
int node_id_counter = 1;

/*Option 1: Mutex Lock*/
void push_mutex() {
    Node *new_node;
    new_node = malloc(sizeof(Node));
    //update top of the stack below
    //assign a unique ID to the new node
    pthread_mutex_lock(&stack_mutex);

    /* assign unique ID while holding the lock to avoid races */
    new_node->node_id = node_id_counter++;
    new_node->next = top;
    top = new_node;

    pthread_mutex_unlock(&stack_mutex);
}
int pop_mutex() {
    pthread_mutex_lock(&stack_mutex);

    if (top == NULL) {
        pthread_mutex_unlock(&stack_mutex);
        return -1; /* empty stack -> sentinel value */
    }

    Node *old_node = top;
    top = old_node->next;
    int id = old_node->node_id;

    pthread_mutex_unlock(&stack_mutex);

    free(old_node);
    return id;
}

/*Option 2: Compare-and-Swap (CAS)*/
void push_cas() {
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));
    //update top of the stack below
    //assign a unique ID to the new node
}
int pop_cas() {
    Node *old_node;
    Node *new_node;
    //update top of the stack below
    return old_node->node_id;
}

/* the thread function */
void *thread_func(void *arg) {
    /* Assign each thread an id so that they are unique in range [0, num_thread -1] */
    Arguments *args = arg;
    int my_id = args->id;
    int opt = args->opt;

    if( opt==0 ) {
        push_mutex();
        push_mutex();
        pop_mutex();
        pop_mutex();
        push_mutex();
    } else {
        push_cas();
        push_cas();
        pop_cas();
        pop_cas();
        push_cas();
    }
    printf("Thread %d: exit\n", my_id);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    //for (int j = 1; j < 7; j++) {
       // num_threads = atoi(argv[j]);
        num_threads = 4;

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
            //pthread_join(...);
        }
        //Print out all remaining nodes in Stack
        printf("Mutex: Remaining nodes \n");
        /*free up resources properly */
        free(workers);
        free(args);

        
        /* Option 2: CAS */
        //pthread_t *workers = malloc(num_threads * sizeof(pthread_t));;
        //Arguments *args = malloc(num_threads * sizeof(Arguments));
        /*
        for (int i = 0; i < num_threads; i++) {
            args[i].id = i;
            args[i].opt = 1;

            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&workers[i], &attr, thread_func, &args[i]);
        }
        for (int i = 0; i < num_threads; i++) {
            //pthread_join(...);
        }
        //Print out all remaining nodes in Stack
        printf("CAS: Remaining nodes \n");
        /*free up resources properly 
        free(workers);
        free(args);*/
    //}

    return 0;
}