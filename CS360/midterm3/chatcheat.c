1. Pthreads basics

Things to be able to write from scratch:

Thread creation loop

pthread_t *tids = calloc(num_threads, sizeof *tids);
for (int i = 0; i < num_threads; i++) {
    // set up per-thread arg here...
    if (pthread_create(&tids[i], NULL, worker, &args[i]) != 0) {
        perror("pthread_create");
        num_threads = i;   // only join created threads
        break;
    }
}

for (int i = 0; i < num_threads; i++) {
    pthread_join(tids[i], NULL);
}


Mutex-protected critical section

From your first worker:

pthread_mutex_lock(&data->lock);
/* modify shared state safely */
pthread_mutex_unlock(&data->lock);


If you can do this pattern in your sleep, you’re golden.

2. Work queue + condition variables

Your prime-checker and thread pool both use the same idea.

Helpers to know:

bool wq_empty(const struct WorkQueue *wq) { return wq->size == 0; }
bool wq_full (const struct WorkQueue *wq) { return wq->size >= wq->capacity; }

bool wq_push(struct WorkQueue *wq, int value) {
    if (wq_full(wq)) return false;
    wq->data[(wq->at + wq->size) % wq->capacity] = value;
    wq->size++;
    return true;
}

int wq_pop(struct WorkQueue *wq) {
    if (wq_empty(wq)) return -1;
    int v = wq->data[wq->at];
    wq->at = (wq->at + 1) % wq->capacity;
    wq->size--;
    return v;
}


Canonical worker loop with cond vars:

void *worker(void *arg)
{
    struct Worker *w = arg;

    for (;;) {
        pthread_mutex_lock(w->lock);

        while (!w->die && wq_empty(w->queue)) {
            pthread_cond_wait(w->not_empty, w->lock);
        }

        if (w->die && wq_empty(w->queue)) {
            pthread_mutex_unlock(w->lock);
            break;
        }

        int v = wq_pop(w->queue);
        pthread_mutex_unlock(w->lock);
        pthread_cond_signal(w->not_full);

        // do work outside the lock
        if (v >= 0) {
            // compute result, append to result list, etc.
        }
    }

    return NULL;
}


If you understand this loop, you basically understand every producer-consumer problem they can throw at you.

3. Dynamic loading (dlopen / dlsym)

Your DSO and plugin drivers show the same pattern:

void *handle = dlopen(libname, RTLD_LAZY);
if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    return 1;
}

SOMEFUNC f = (SOMEFUNC)dlsym(handle, "symbol_name");
if (!f) {
    fprintf(stderr, "%s\n", dlerror());
    dlclose(handle);
    return 1;
}


You should be comfortable with:

Building library names (lib prefix, .so suffix, maybe ./).

Getting function pointers with dlsym.

Checking errors and calling dlclose.

4. Parsing name=value settings

Used in your plugin host:

char name[256] = "";
char value[256] = "";

char *eq = strchr(arg, '=');
if (eq) {
    int n = eq - arg;
    strncpy(name, arg, n);
    name[n] = '\0';
    strcpy(value, eq + 1);
} else {
    strcpy(name, arg);
    value[0] = '\0';
}


This pattern is super reusable any time they give you simple “key=value” command-line options.

5. Simple plugins / string transforms

You already wrote several; the core patterns to know:

Uppercasing every N-th alpha char (stride)

static int stride = 1;

PluginResult setting(const char name[], const char value[]) {
    if (strcmp(name, "stride") == 0) {
        char *end;
        long s = strtol(value, &end, 10);
        if (*end != '\0' || s <= 0) return PR_FAILED;
        stride = (int)s;
        return PR_SUCCESS;
    }
    return PR_FAILED;
}

PluginResult transform(char *data, int *size) {
    int count = 0;
    for (int i = 0; i < *size; i++) {
        if (isalpha((unsigned char)data[i])) {
            if (count % stride == 0) {
                data[i] = toupper((unsigned char)data[i]);
            }
            count++;
        }
    }
    return PR_SUCCESS;
}


Reverse a buffer in place

PluginResult transform(char *data, int *size) {
    int left = 0;
    int right = *size - 1;
    while (left < right) {
        char tmp = data[left];
        data[left] = data[right];
        data[right] = tmp;
        left++;
        right--;
    }
    return PR_SUCCESS;
}


Substitute based on mapping arrays

You already nailed this with upper[] and lower[]: compute index c - 'A' or c - 'a', check 0–25, then map.

6. Thread pool pattern

Your tpool.c is basically a generalized version of the prime worker queue:

Core responsibilities of thread_pool_open you should know:

Allocate thread_pool_t.

Set nThread, done = false.

Init queue indices and capacity.

Initialize mutex, workCond, doneCond.

Create worker threads with pthread_create.

Core responsibilities of thread_pool_execute:

Allocate results[nWork].

For each work item:

Lock, enqueue job_t (value + index + executor), unlock, signal/broadcast.

Wait until resultC == nWork using doneCond.

Return the results array.

Core responsibilities of thread_pool_close:

Set done = true, broadcast workCond.

Join all worker threads.

Destroy mutex/conds, free memory.

You don’t need to memorize every line, but you do need to be able to reconstruct that logic under exam conditions.

7. Hashing helpers

Your hash32 and hash64 are standard FNV-style loops:

uint64_t hash32(int fd) {
    uint32_t h = 2166136261U;
    uint32_t p = 16777619U;
    unsigned char buf[4096];
    ssize_t n;

    lseek(fd, 0, SEEK_SET);
    while ((n = read(fd, buf, sizeof buf)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            h = (h ^ buf[i]) * p;
        }
    }
    return (uint64_t)h;
}


Useful pattern to recognize: read in a loop, update state per byte.