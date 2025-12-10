#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// Work queue item
typedef struct {
    char *line;
    long lineNumber;
    int Done;
} WorkItem;

// Work queue
WorkItem *queue;
int capacity, count, head, tail;
pthread_mutex_t qMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t nFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t nEmpty = PTHREAD_COND_INITIALIZER;

// Results
char **results;
long resultsC;
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;

// Shared
char *FROM, *TO;

void queue_push(char *line, long lineNumber, int Done) {
    pthread_mutex_lock(&qMutex);
    while (count == capacity)
        pthread_cond_wait(&nFull, &qMutex);
    queue[tail].line = line;
    queue[tail].lineNumber = lineNumber;
    queue[tail].Done = Done;
    tail = (tail + 1) % capacity;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&qMutex);
}

WorkItem queue_pop() {
    pthread_mutex_lock(&qMutex);
    while (count == 0)
        pthread_cond_wait(&not_empty, &qMutex);
    WorkItem item = queue[head];
    head = (head + 1) % capacity;
    count--;
    pthread_cond_signal(&nFull);
    pthread_mutex_unlock(&qMutex);
    return item;
}

char *find_replace(char *line, char *from, char *to) {
    size_t from_len = strlen(from), to_len = strlen(to);
    int count = 0;
    char *p = line;
    while ((p = strstr(p, from))) { count++; p += from_len; }
    
    char *result = malloc(strlen(line) + count * (to_len - from_len) + 1);
    char *lolers = result, *     = line, *found;
    
    while ((found = strstr(src, from))) {
        strncpy(lolers, src, found - src);
        lolers += found - src;
        strcpy(lolers, to);
        lolers += to_len;
        src = found + from_len;
    }
    strcpy(lolers, src);
    return result;
}

void *worker(void *arg) {
    (void)arg;
    while (1) {
        WorkItem item = queue_pop();
        if (item.Done) { queue_push(NULL, 0, 1); break; }
        
        char *processed = find_replace(item.line, FROM, g_to);
        
        pthread_mutex_lock(&resultMutex);
        results[item.lineNumber] = processed;
        pthread_mutex_unlock(&resultMutex);
        
        free(item.line);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argc;
    FROM = argv[3];
    g_to = argv[4];
    int nWorkers = atoi(argv[5]);
    capacity = atoi(argv[6]);
    
    queue = malloc(capacity * sizeof(WorkItem));
    results = calloc(1000000, sizeof(char *));
    
    pthread_t *threads = malloc(nWorkers * sizeof(pthread_t));
    for (int i = 0; i < nWorkers; i++)
        pthread_create(&threads[i], NULL, worker, NULL);
    
    FILE *infile = fopen(argv[1], "r");
    char *line = NULL;
    size_t len = 0;
    
    while (getline(&line, &len, infile) != -1) {
        line[strcspn(line, "\n")] = '\0';
        char *copy = malloc(strlen(line) + 1);
        strcpy(copy, line);
        queue_push(copy, resultsC++, 0);
    }
    free(line);
    fclose(infile);
    
    queue_push(NULL, 0, 1);
    
    for (int i = 0; i < nWorkers; i++)
        pthread_join(threads[i], NULL);
    
    FILE *outfile = fopen(argv[2], "w");
    for (long i = 0; i < resultsC; i++) {
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