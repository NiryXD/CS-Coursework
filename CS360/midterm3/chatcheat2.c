1. Safe integer parsing helper (for argc/argv)

You’ve done sscanf a lot already. Here’s a reusable wrapper:

// Parse an integer from a string with basic validation.
// Returns true on success, false on failure.
static bool parse_int(const char *s, int *out)
{
    char extra;
    // %d reads an int; " %c" would catch leftover junk, so we use sscanf carefully.
    if (sscanf(s, "%d %c", out, &extra) != 1) {
        return false;  // either no number or extra garbage after it
    }
    return true;
}


Usage:

int num_workers;
if (!parse_int(argv[1], &num_workers) || num_workers <= 0) {
    fprintf(stderr, "Invalid worker count '%s'\n", argv[1]);
    return 1;
}

2. “Launch N threads” helper using one shared arg

You’ve written this pattern multiple times; here’s a generic helper:

// Launch n threads that all run the same worker function with the same arg.
// Returns 0 on success, non-zero on failure.
static int spawn_threads(pthread_t *tids, int n,
                         void *(*worker)(void *), void *arg)
{
    for (int i = 0; i < n; i++) {
        if (pthread_create(&tids[i], NULL, worker, arg) != 0) {
            // If one fails, join previous ones and signal error.
            for (int j = 0; j < i; j++) {
                pthread_join(tids[j], NULL);
            }
            return -1;
        }
    }
    return 0;
}


Usage:

pthread_t *tids = calloc(num_workers, sizeof *tids);
// ... initialize shared arg struct ...
if (spawn_threads(tids, num_workers, worker, &shared) != 0) {
    fprintf(stderr, "Failed to create threads\n");
    free(tids);
    return 1;
}
for (int i = 0; i < num_workers; i++) {
    pthread_join(tids[i], NULL);
}


If each thread needs a different argument, you already know that pattern (array of structs, pass &args[i]).

3. WorkQueue init/destroy helpers (for your prime code)

You already have wq_push, wq_pop, wq_full, wq_empty. Add these so you don’t have to re-write the setup/cleanup logic:

// Initialize a WorkQueue with the given capacity.
// Returns true on success, false on failure.
static bool wq_init(struct WorkQueue *wq, int capacity)
{
    wq->data = calloc(capacity, sizeof *wq->data);
    if (!wq->data) {
        return false;
    }
    wq->capacity = capacity;
    wq->size = 0;
    wq->at = 0;
    return true;
}

// Free resources owned by a WorkQueue.
static void wq_destroy(struct WorkQueue *wq)
{
    free(wq->data);
    wq->data = NULL;
    wq->capacity = 0;
    wq->size = 0;
    wq->at = 0;
}


Usage (this replaces manual calloc/free):

struct WorkQueue queue = {0};
if (!wq_init(&queue, work_queue_size)) {
    perror("wq_init");
    free(workers);
    return 1;
}

// ... use queue with wq_push / wq_pop ...

wq_destroy(&queue);

4. Thread-safe push into your ResultList

In the prime code, you do this pattern:

Allocate struct Result

Fill it

Lock, push to head, unlock

Here’s a helper so you can call it in any worker:

// Thread-safe push to the front of ResultList.
// Caller passes the shared mutex (the same one guarding the queue/results).
static bool results_push_front(struct ResultList *list,
                               const struct Data *d,
                               pthread_mutex_t *lock)
{
    struct Result *r = malloc(sizeof *r);
    if (!r) return false;

    r->data = *d;

    pthread_mutex_lock(lock);
    r->next = list->head;
    list->head = r;
    pthread_mutex_unlock(lock);

    return true;
}


Usage inside your worker:

if (v >= 0) {
    struct Data d = { .value = v, .is_prime = awful_is_prime(v) };
    results_push_front(w->results, &d, w->lock);
}


Now the worker logic is cleaner and you can reuse this pattern for any “append to shared list” kind of exam problem.

5. Generic name=value parser (you already kind of wrote it)

You’ve written this logic a few times; here’s a clean reusable version:

// Split "name=value" into name and value buffers.
// If no '=' is present, name = whole string, value = "".
static void parse_name_value(const char *arg,
                             char *name, size_t name_sz,
                             char *value, size_t value_sz)
{
    const char *eq = strchr(arg, '=');
    if (!eq) {
        // no '=': entire arg is name
        snprintf(name, name_sz, "%s", arg);
        if (value_sz > 0) {
            value[0] = '\0';
        }
        return;
    }

    size_t nlen = (size_t)(eq - arg);
    if (nlen >= name_sz) nlen = name_sz - 1;
    memcpy(name, arg, nlen);
    name[nlen] = '\0';

    snprintf(value, value_sz, "%s", eq + 1);
}


Usage in plugin driver or DSO setting:

char name[256], value[256];
parse_name_value(argv[i], name, sizeof name, value, sizeof value);
PluginResult pr = settingFunction(name, value);

6. DSO loader helper (for your DSO / plugin code)

You’ve already hand-coded all the variations of dlopen. Here’s a reusable function you can adapt (simpler version than your long chain):

// Try to load a shared object by trying a few common variations.
// Returns a handle on success, or NULL on failure.
static void *load_dso_variants(const char *stem)
{
    char path[512];

    // 1) as given
    void *h = dlopen(stem, RTLD_LAZY);
    if (h) return h;

    // 2) ./stem
    snprintf(path, sizeof path, "./%s", stem);
    h = dlopen(path, RTLD_LAZY);
    if (h) return h;

    // 3) stem.so
    snprintf(path, sizeof path, "%s.so", stem);
    h = dlopen(path, RTLD_LAZY);
    if (h) return h;

    // 4) ./stem.so
    snprintf(path, sizeof path, "./%s.so", stem);
    h = dlopen(path, RTLD_LAZY);
    if (h) return h;

    // 5) libstem.so
    snprintf(path, sizeof path, "lib%s.so", stem);
    h = dlopen(path, RTLD_LAZY);
    if (h) return h;

    // 6) ./libstem.so
    snprintf(path, sizeof path, "./lib%s.so", stem);
    h = dlopen(path, RTLD_LAZY);
    if (h) return h;

    return NULL;
}


Usage:

void *handle = load_dso_variants(argv[2]);
if (!handle) {
    fprintf(stderr, "Failed to load DSO: %s\n", dlerror());
    return 1;
}


Then you reuse your dlsym pattern like you already do.

7. mmap helper (your plugin driver uses this logic)

You use open + fstat + mmap a lot. Here’s a reusable helper:

// Map an entire file read/write. Returns pointer, sets *size to file size.
// On failure, returns MAP_FAILED and *size is undefined.
static char *map_file_rw(const char *filename, int *size_out, int *fd_out)
{
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror(filename);
        return MAP_FAILED;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return MAP_FAILED;
    }

    if (st.st_size == 0) {
        // empty file: still return a valid fd, but no mapping
        *size_out = 0;
        *fd_out = fd;
        return NULL;
    }

    char *data = mmap(NULL, st.st_size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return MAP_FAILED;
    }

    *size_out = (int)st.st_size;
    *fd_out = fd;
    return data;
}


Usage:

int size, fd;
char *data = map_file_rw(fn, &size, &fd);
if (data == MAP_FAILED) return 1;

// use data/size, e.g. transform(data, &size);

if (data && size > 0) {
    munmap(data, size);
}
close(fd);

8. “Guarded critical section” mini macro

This one just makes your mutex usage less error-prone:

// Execute a block of code under a lock.
// Example usage:
//
// WITH_LOCK(&data->lock, {
//     data->result += t->x * t->y + t->z;
// });
//
#define WITH_LOCK(mtx, block) \
    do { \
        pthread_mutex_lock((mtx)); \
        block; \
        pthread_mutex_unlock((mtx)); \
    } while (0)


Usage in your first worker:

void *worker(void *arg)
{
    struct Data *data = (struct Data *)arg;
    struct List *t;

    WITH_LOCK(&data->lock, {
        t = data->work_list;
        data->work_list = t->next;
        data->result += t->x * t->y + t->z;
    });

    free(t);
    return NULL;
}


Same logic, but less typing and harder to forget unlock.

9. Small “queue drain” helper (you used this with usleep)

You had this pattern:

while (1) {
    pthread_mutex_lock(&lock);
    bool empty = wq_empty(&queue);
    pthread_mutex_unlock(&lock);
    if (empty) break;
    usleep(100000);
}


Here’s a helper:

// Busy-wait (with sleep) until the queue becomes empty.
// Be careful: just a simple exam-style helper, not ideal for production.
static void wait_until_empty(struct WorkQueue *wq,
                             pthread_mutex_t *lock)
{
    for (;;) {
        pthread_mutex_lock(lock);
        bool empty = wq_empty(wq);
        pthread_mutex_unlock(lock);
        if (empty) break;
        usleep(100000); // 0.1 s
    }
}


Usage:

wait_until_empty(&queue, &lock);

10. Tiny “thread pool enqueue one job” helper

You’ve already written full thread_pool_execute. If an exam only asks you to “submit one job” or you want to reuse the pattern, this is nice:

// Internal helper: push a single job into the thread pool queue
// Assumes caller holds pool->mutex and that queue is not full.
static void pool_enqueue_job(thread_pool_t *pool,
                             int workVal, int index,
                             Executor executor)
{
    job_t job;
    job.workVal = workVal;
    job.index = index;
    job.executor = executor;

    pool->jobQ[pool->Qback] = job;
    pool->Qback = (pool->Qback + 1) % pool->Qcapacity;
    pool->Qsize++;
}


Then your thread_pool_execute inner loop simplifies to:

pthread_mutex_lock(&pool->mutex);
while (pool->Qsize < pool->Qcapacity && i < nWork) {
    pool_enqueue_job(pool, workVal[i], i, executor);
    i++;
}
pthread_cond_broadcast(&pool->workCond);
pthread_mutex_unlock(&pool->mutex);