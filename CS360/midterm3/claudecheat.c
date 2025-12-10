1. Basic Thread Worker Pattern
cvoid *worker(void *arg) {
    struct Data *data = (struct Data *)arg;
    
    pthread_mutex_lock(&data->lock);
    // Critical section - do work here
    pthread_mutex_unlock(&data->lock);
    
    return NULL;
}
2. Producer-Consumer Pattern
c// Producer
pthread_mutex_lock(&lock);
while (queue_full()) {
    pthread_cond_wait(&not_full, &lock);
}
// Add item to queue
pthread_mutex_unlock(&lock);
pthread_cond_signal(&not_empty);

// Consumer
pthread_mutex_lock(&lock);
while (queue_empty()) {
    pthread_cond_wait(&not_empty, &lock);
}
// Remove item from queue
pthread_mutex_unlock(&lock);
pthread_cond_signal(&not_full);
3. Dynamic Library Loading Pattern
cvoid *load_library(const char *name) {
    char path[256];
    void *handle = NULL;
    
    // Try different variations
    snprintf(path, sizeof(path), "./%s", name);
    handle = dlopen(path, RTLD_LAZY);
    
    if (!handle) {
        snprintf(path, sizeof(path), "./lib%s.so", name);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    return handle;
}
4. Thread Pool Creation
cpthread_t *create_threads(int num_threads, void *(*func)(void*), void *data) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) return NULL;
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, func, &data[i]) != 0) {
            // Handle error - join created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            return NULL;
        }
    }
    return threads;
}
5. Queue Operations
c// Circular queue push
bool queue_push(struct Queue *q, int value) {
    if (q->size >= q->capacity) return false;
    q->data[(q->at + q->size) % q->capacity] = value;
    q->size++;
    return true;
}

// Circular queue pop
int queue_pop(struct Queue *q) {
    if (q->size == 0) return -1;
    int ret = q->data[q->at];
    q->at = (q->at + 1) % q->capacity;
    q->size--;
    return ret;
}
6. Memory-Mapped File Pattern
cchar *map_file(const char *filename, int *size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return NULL;
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }
    
    *size = st.st_size;
    char *data = mmap(NULL, *size, PROT_READ | PROT_WRITE, 
                      MAP_PRIVATE, fd, 0);
    close(fd);
    
    if (data == MAP_FAILED) return NULL;
    return data;
}
7. String Reversal (In-Place)
cvoid reverse_string(char *str, int len) {
    int left = 0, right = len - 1;
    while (left < right) {
        char temp = str[left];
        str[left] = str[right];
        str[right] = temp;
        left++;
        right--;
    }
}
8. Thread Synchronization Cleanup
cvoid cleanup_sync(pthread_mutex_t *mutex, pthread_cond_t *cond) {
    if (mutex) pthread_mutex_destroy(mutex);
    if (cond) pthread_cond_destroy(cond);
}
9. Safe Thread Join Pattern
cvoid join_all_threads(pthread_t *threads, int count) {
    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }
}
10. Plugin Function Signatures
c// Common plugin patterns
typedef void (*INIT)(void);
typedef int (*TRANSFORM)(char *data, int *size);
typedef int (*SETTING)(const char *name, const char *value);

// Load and execute plugin
void *func = dlsym(handle, "function_name");
if (!func) {
    fprintf(stderr, "Error: %s\n", dlerror());
    dlclose(handle);
    return -1;
}