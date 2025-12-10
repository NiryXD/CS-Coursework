#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <errno.h>
#include <signal.h>
#include <time.h>

// Steps so I don't need to look at the paper
// Multi-threaded "find-and-replace" C program

// 1. A total of 6 user-supplied command line arguments will nbe given to your main()
// a. argv[1]: input file name
// b. argv[2]: output file name
// c. argv[3]: from string
// d. argv[4]: to string 
// e. argv[5]: number of workers in the thread pool 
// f. argv[6]: number of values that can be in the work queue at once 

typedef struct {
    char *line;
    long lineNumber;
    int Done;
} WorkItem;

WorkItem *queue;
    int capacity, count, head, tail;
    pthread_mutex_t qMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t nFull = PTHREAD_COND_INITIALIZER;
    pthread_cond_t nEmpty = PTHREAD_COND_INITIALIZER;

char **results;
long resultsC;
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;

char *FROM, *TO;

// 2. Create a work queue, mutex, and conditional variables necessary for your work queue.

static void x_pthread_create(pthread_t *t,
                             const pthread_attr_t *attr,
                             void *(*start_routine)(void *),
                             void *arg)
{
    int rc = pthread_create(t, attr, start_routine, arg);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_pthread_join(pthread_t t, void **retval)
{
    int rc = pthread_join(t, retval);
    if (rc != 0) {
        fprintf(stderr, "pthread_join: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_init(pthread_mutex_t *m)
{
    int rc = pthread_mutex_init(m, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_init: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_lock(pthread_mutex_t *m)
{
    int rc = pthread_mutex_lock(m);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_lock: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_unlock(pthread_mutex_t *m)
{
    int rc = pthread_mutex_unlock(m);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_init(pthread_cond_t *c)
{
    int rc = pthread_cond_init(c, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
    int rc = pthread_cond_wait(c, m);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_wait: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_signal(pthread_cond_t *c)
{
    int rc = pthread_cond_signal(c);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_signal: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_broadcast(pthread_cond_t *c)
{
    int rc = pthread_cond_broadcast(c);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_broadcast: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

    void qPush(char *line, long lineNumber, int Done) {
        pthread_mutex_lock(&qMutex);
        while (count == capacity)
        pthread_cond_wait(&nFull, &qMutex);
        queue[tail].line = line;
        queue[tail].lineNumber = lineNumber;
        queue[tail].Done = Done;
        tail = (tail + 1) % capacity;
        count++;
        pthread_cond_signal(&nEmpty);
        pthread_mutex_unlock(&qMutex);

    }

    WorkItem qPop() {
        pthread_mutex_lock(&qMutex);
        while (count == 0)
        pthread_cond_wait(&nEmpty, &qMutex);
        WorkItem item = queue[head];
        head = (head + 1) % capacity;
        count--;
        pthread_cond_signal(&nFull);
        pthread_mutex_unlock(&qMutex);
        return item;
    }

    char *replaceFind(char *line, char *from, char *to) {
        size_t fromLength = strlen(from), toLength = strlen(to);
        int count = 0;
        char *p = line;

        while ((p = strstr(p, from))) {
            count++;
            p += fromLength;
        }

        char *result = malloc(strlen(line) + count * (toLength - fromLength) + 1);
        char *lolers = result, *src = line, *found;

        while ((found = strstr(src, from))) {
            strncpy(lolers, src, found - src);
            lolers += found - src;
            strcpy(lolers, to);
            lolers += toLength;
            src = found + fromLength;
        }
        strcpy(lolers, src);
        return result;
    }

// 3. Create thread pool. The thread pool will only have argv[5] number of workers.

void *worker(void *arg) {
    (void)arg;
    while (1) {
        WorkItem item = qPop();
        if (item.Done) {
            qPush(NULL, 0, 1);
            break;
        }

        char *yesSir = replaceFind(item.line, FROM, TO);

        pthread_mutex_lock(&resultMutex);
        results[item.lineNumber] = yesSir;
        pthread_mutex_unlock(&resultMutex);

        free(item.line);
    }


    return NULL;
}

// 4. Read each line from the input file (argv[1]) and fill up the work queue
// Assume there could be billions of lines. Do not read all lines at once

// 5. Have your workers find and update every argv[3] in the line and replace it with argv[4].

static void line_write(FILE *fp, const char *s)
{
    if (fputs(s, fp) == EOF) {
        perror("fputs");
        exit(EXIT_FAILURE);
    }
}  

int main(int argc, char *argv[])
{

    (void)argc;
    FROM = argv[3];
    TO = argv[4];
    int nWorkers = atoi(argv[5]);
    capacity = atoi(argv[6]);

    queue = malloc(capacity * sizeof(WorkItem));
    results = calloc(1000000000, sizeof(char *));

    pthread_t *thread = malloc(nWorkers * sizeof(pthread_t));

    for (int i = 0; i < nWorkers; i++)
    pthread_create(&thread[i], NULL, worker, NULL );

    FILE *infile = fopen(argv[1], "read");
    char *line = NULL;
    size_t lenght = 0;

    while (getline(&line, &lenght, infile) != -1) {
        line[strcspn(line, "\n")] = '\0';
        char *copy = malloc(strlen(line) + 1);
        strcpy(copy, line);
        qPush(copy, resultsC++, 0);
    }

    free(line);
    fclose(infile);

    qPush(NULL, 0 ,1);

    for (int i = 0; i < nWorkers; i++)
    pthread_join(thread[i], NULL);

    // 6. Store the new lines into the outputfile (argv[2]).

    FILE *outfile = fopen(argv[2], "write");
    for (long i = 0; i < resultsC; i++) {
        fputs(results[i], outfile);
        fputc('\n', outfile);
        free(results[i]);
    }

    // 7. Close all files and free all allocated resources

    fclose(outfile);
    free(results);
    free(queue);
    free(thread);
    return 0;   
}

// I need a 75 to pass! I got this! 
// TREMENDOUS LOCK IN, LETS GOOOOO
