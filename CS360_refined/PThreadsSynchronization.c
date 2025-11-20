#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

struct List {
    int x;
    int y;
    int z;
    struct List *next;
};

struct Data {
    int result;
    struct List *work_list;
    pthread_mutex_t lock;
};

void *worker(void *arg)
{
    struct Data *data = (struct Data *)arg;
    struct List *t;
    
    pthread_mutex_lock(&data->lock);
    t = data->work_list;
    data->work_list = t->next;
    data->result += t->x * t->y + t->z;
    pthread_mutex_unlock(&data->lock);
    free(t);

    return NULL;
}
