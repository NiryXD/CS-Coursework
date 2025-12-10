#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// Work queue item
typedef struct {
    char *line;
    long line_num;
    int done;
} WorkItem;

// Work queue
WorkItem *queue;
int queue_cap, queue_count, queue_head, queue_tail;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

// Results
char **results;
long result_count;
pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

// Shared
char *g_from, *g_to;

void queue_push(char *line, long line_num, int done) {
    pthread_mutex_lock(&queue_mutex);
    while (queue_count == queue_cap)
        pthread_cond_wait(&not_full, &queue_mutex);
    queue[queue_tail].line = line;
    queue[queue_tail].line_num = line_num;
    queue[queue_tail].done = done;
    queue_tail = (queue_tail + 1) % queue_cap;
    queue_count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

WorkItem queue_pop() {
    pthread_mutex_lock(&queue_mutex);
    while (queue_count == 0)
        pthread_cond_wait(&not_empty, &queue_mutex);
    WorkItem item = queue[queue_head];
    queue_head = (queue_head + 1) % queue_cap;
    queue_count--;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&queue_mutex);
    return item;
}

char *find_replace(char *line, char *from, char *to) {
    size_t from_len = strlen(from), to_len = strlen(to);
    int count = 0;
    char *p = line;
    while ((p = strstr(p, from))) { count++; p += from_len; }
    
    char *result = malloc(strlen(line) + count * (to_len - from_len) + 1);
    char *dst = result, *src = line, *found;
    
    while ((found = strstr(src, from))) {
        strncpy(dst, src, found - src);
        dst += found - src;
        strcpy(dst, to);
        dst += to_len;
        src = found + from_len;
    }
    strcpy(dst, src);
    return result;
}

void *worker(void *arg) {
    (void)arg;
    while (1) {
        WorkItem item = queue_pop();
        if (item.done) { queue_push(NULL, 0, 1); break; }
        
        char *processed = find_replace(item.line, g_from, g_to);
        
        pthread_mutex_lock(&result_mutex);
        results[item.line_num] = processed;
        pthread_mutex_unlock(&result_mutex);
        
        free(item.line);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argc;
    g_from = argv[3];
    g_to = argv[4];
    int num_workers = atoi(argv[5]);
    queue_cap = atoi(argv[6]);
    
    queue = malloc(queue_cap * sizeof(WorkItem));
    results = calloc(1000000, sizeof(char *));
    
    pthread_t *threads = malloc(num_workers * sizeof(pthread_t));
    for (int i = 0; i < num_workers; i++)
        pthread_create(&threads[i], NULL, worker, NULL);
    
    FILE *infile = fopen(argv[1], "r");
    char *line = NULL;
    size_t len = 0;
    
    while (getline(&line, &len, infile) != -1) {
        line[strcspn(line, "\n")] = '\0';
        char *copy = malloc(strlen(line) + 1);
        strcpy(copy, line);
        queue_push(copy, result_count++, 0);
    }
    free(line);
    fclose(infile);
    
    queue_push(NULL, 0, 1);
    
    for (int i = 0; i < num_workers; i++)
        pthread_join(threads[i], NULL);
    
    FILE *outfile = fopen(argv[2], "w");
    for (long i = 0; i < result_count; i++) {
        fputs(results[i], outfile);
        fputc('\n', outfile);
        free(results[i]);
    }
    fclose(outfile);
    
    free(results);
    free(queue);
    free(threads);
    return 0;
}