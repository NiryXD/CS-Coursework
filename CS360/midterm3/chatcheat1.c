pthread_create() creates a new thread. Its function signature is:

int pthread_create(
    pthread_t *thread,          // OUT: stores the thread ID
    const pthread_attr_t *attr, // IN: thread attributes (or NULL for default)
    void *(*start_routine)(void *), // IN: function the thread will run
    void *arg                   // IN: argument passed to the function
);

Parameters Breakdown
Parameter	Type	Purpose
pthread_t *thread	pointer to a thread ID variable	Where the ID of the new thread is stored after creation.
const pthread_attr_t *attr	pointer to attributes struct	Controls settings like stack size, scheduling. Use NULL for default settings.
void *(*start_routine)(void *)	pointer to a function	The function the thread executes. Must match signature: void *func(void *arg).
void *arg	pointer (any type)	Argument passed to the start_routine. Can be NULL or cast your data.
Example
#include <pthread.h>
#include <stdio.h>

void *worker(void *arg) {
    int *x = (int *)arg;
    printf("Thread got value: %d\n", *x);
    return NULL;
}

int main() {
    pthread_t tid;
    int value = 5;

    pthread_create(&tid, NULL, worker, &value);
    pthread_join(tid, NULL); // Wait for thread to finish
    return 0;
}

Important Notes

The function passed must return void * and accept void * as its only parameter.

Use pthread_join() to wait for the thread to finish.

