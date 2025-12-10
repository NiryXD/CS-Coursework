// KEYWORDS: MQUEUE IPC
// Creates a POSIX message queue (writer side) and returns the
// mqd_t descriptor. If creation fails the program exits.
// Plain-English: "Make a named mailbox the parent can write to so
// workers can read tasks from it."
mqd_t create_mqueue(const char *name, size_t msgsize) {
    mq_unlink(name);  // Clean up any existing
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = msgsize;
    attr.mq_curmsgs = 0;
    
    mqd_t mq = mq_open(name, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }
    return mq;
}

// KEYWORDS: MQUEUE IPC
// Opens an existing POSIX message queue for reading. Exits on error.
// Plain-English: "Connect to the named mailbox so this process can
// receive messages (tasks)."
mqd_t open_mqueue_read(const char *name) {
    mqd_t mq = mq_open(name, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }
    return mq;
}

// KEYWORDS: SEMAPHORE SYNCHRONIZATION
// Creates a named binary semaphore (acts like a mutex). Initial
// value is 1 so the first waiter acquires immediately. Exits on
// error.
// Plain-English: "Create a system-wide lock other processes can use
// to protect shared data." 
sem_t *create_semaphore(const char *name) {
    sem_unlink(name);  // Clean up any existing
    
    sem_t *sem = sem_open(name, O_CREAT, 0666, 1);  // Initial value = 1
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return sem;
}

// KEYWORDS: SEMAPHORE SYNCHRONIZATION
// Open an existing named semaphore created by another process.
// Plain-English: "Connect to the system lock someone else created."
sem_t *open_semaphore(const char *name) {
    sem_t *sem = sem_open(name, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return sem;
}

// KEYWORDS: SEMAPHORE COUNTING
// Create a named counting semaphore with a specified initial value.
// Useful for producer/consumer situations where you count available
// slots or items.
// Plain-English: "Create a counter-based lock that can track N
// resources instead of just locked/unlocked."
sem_t *create_counting_semaphore(const char *name, int initial_value) {
    sem_unlink(name);
    
    sem_t *sem = sem_open(name, O_CREAT, 0666, initial_value);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    return sem;
}

// KEYWORDS: SHM SHARED_MEMORY MMAP FTRUNCATE
// Create and map a POSIX shared memory object of the requested size
// and return a pointer to the mapped region. Also returns the
// underlying file descriptor via shm_fd_out if provided.
// Plain-English: "Allocate a named chunk of memory that other
// processes can open by name and access like normal memory."
void *create_shared_memory(const char *name, size_t size, int *shm_fd_out) {
    shm_unlink(name);  // Clean up any existing
    
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        exit(1);
    }
    
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(1);
    }
    
    if (shm_fd_out) *shm_fd_out = shm_fd;
    return ptr;
}

// KEYWORDS: SHM SHARED_MEMORY MMAP
// Open and mmap an existing shared memory object created elsewhere.
// Plain-English: "Connect to the named shared memory so we can read
// and write it." Note: caller must know the object's size.
void *open_shared_memory(const char *name, size_t size, int *shm_fd_out) {
    int shm_fd = shm_open(name, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(1);
    }
    
    if (shm_fd_out) *shm_fd_out = shm_fd;
    return ptr;
}

// KEYWORDS: SHM FTRUNCATE MMAP REMAP
// Grow an existing shared memory object from old_size to new_size.
// The function unmaps the old mapping, calls ftruncate, and remaps
// the object. Returns the new mapping or NULL on failure.
// Plain-English: "Make the named shared memory bigger and re-open
// it so the new bytes are available." Caller must ensure nobody is
// accessing the old mapping while this happens.
void *grow_shared_memory(int shm_fd, void *old_ptr, size_t old_size, size_t new_size) {
    munmap(old_ptr, old_size);
    
    if (ftruncate(shm_fd, new_size) == -1) {
        perror("ftruncate");
        return NULL;
    }
    
    void *new_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (new_ptr == MAP_FAILED) {
        perror("mmap after grow");
        return NULL;
    }
    
    return new_ptr;
}

// KEYWORDS: FORK EXECV PROCESS_CONTROL
// Fork and exec multiple worker processes. On success returns a
// heap-allocated array of PIDs (caller must free). If any fork
// fails previously forked children are terminated.
// Plain-English: "Start N worker processes that run the given
// program with the provided arguments."
pid_t *fork_workers(int num_workers, const char *worker_program, char *args[]) {
    pid_t *pids = malloc(sizeof(pid_t) * num_workers);
    if (!pids) {
        perror("malloc");
        exit(1);
    }
    
    for (int i = 0; i < num_workers; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            // Kill already-forked children
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            exit(1);
        }
        
        if (pids[i] == 0) {
            // Child process
            execv(worker_program, args);
            perror("execv");
            exit(1);
        }
    }
    
    return pids;
}

// KEYWORDS: WAITPID PROCESS_CONTROL
// Wait for an array of child PIDs to finish and print a warning if
// any exited with a non-zero status.
// Plain-English: "Block until each worker exits and report failures."
void wait_for_workers(pid_t *pids, int num_workers) {
    for (int i = 0; i < num_workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Worker %d exited with code %d\n", i, WEXITSTATUS(status));
        }
    }
}

// KEYWORDS: FORK EXECV WAIT
// Fork, exec a program, wait for it to finish, and return its exit
// code (or -1 on error).
// Plain-English: "Run a program synchronously and give me its exit
// status. Useful for helper tasks that should run and finish before
// continuing."
int fork_exec_wait(const char *program, char *args[]) {
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        execv(program, args);
        perror("execv");
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

// KEYWORDS: PIPE IPC
// Create a unidirectional pipe and return read/write file
// descriptors via the provided pointers.
// Plain-English: "Give me two file descriptors I can use to stream
// bytes between two processes (parent/child)."
void create_pipe(int *read_fd, int *write_fd) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    *read_fd = pipefd[0];
    *write_fd = pipefd[1];
}

// KEYWORDS: PIPE PARENT_CHILD
// Helper to set up pipe endpoints in the parent: close the unused
// write end so reads see EOF when all writers close.
void setup_pipe_parent(int read_fd, int write_fd) {
    close(write_fd);
}

// KEYWORDS: PIPE PARENT_CHILD
// Helper to set up pipe endpoints in the child: close the unused
// read end so the child can write to the pipe.
void setup_pipe_child(int read_fd, int write_fd) {
    close(read_fd);
}

// KEYWORDS: THREADS PTHREAD
// Create a group of pthreads running 'func' with the provided data
// pointers (one per thread). Returns a heap array of pthread_t that
// the caller must free. Exits on failure to create threads.
// Plain-English: "Spawn N threads; each thread gets data[i] as its
// argument and runs func(data[i])."
pthread_t *create_threads(int num_threads, void *(*func)(void*), void *data[]) {
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    if (!threads) {
        perror("malloc");
        exit(1);
    }
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, func, data[i]) != 0) {
            perror("pthread_create");
            free(threads);
            exit(1);
        }
    }
    
    return threads;
}

// KEYWORDS: THREADS PTHREAD JOIN
// Join an array of pthreads (wait for each to finish).
// Plain-English: "Block until all spawned threads are done." 
void join_threads(pthread_t *threads, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// KEYWORDS: FILE IO READ
// Read the whole file into a newly malloc'd buffer and return it.
// The returned buffer is NUL-terminated to make string operations
// safe. Caller must free the buffer. On failure returns NULL.
char *read_file_contents(const char *filename, size_t *size_out) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror(filename);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, size, f);
    buffer[bytes_read] = '\0';
    
    fclose(f);
    
    if (size_out) *size_out = bytes_read;
    return buffer;
}

// KEYWORDS: FILE IO COUNT LINES
// Count newline characters in a text file. Plain-English: "How many
// lines are in this file?" Returns 0 on error or empty file.
size_t count_lines(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    
    size_t count = 0;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') count++;
    }
    
    fclose(f);
    return count;
}

// KEYWORDS: FILE IO COUNT WORDS
// Count words in a file using a simple whitespace-based definition.
// Plain-English: "Roughly how many words are in this file?"
size_t count_words(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    
    size_t count = 0;
    int in_word = 0;
    int c;
    
    while ((c = fgetc(f)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
    }
    
    fclose(f);
    return count;
}

// KEYWORDS: IPC CLEANUP MQUEUE SEMAPHORE SHM
// Remove named IPC objects (message queue, semaphore, shared
// memory). This only removes the name from the system namespace; any
// open descriptors must still be closed separately.
void cleanup_ipc(const char *mq_name, const char *sem_name, const char *shm_name) {
    if (mq_name) mq_unlink(mq_name);
    if (sem_name) sem_unlink(sem_name);
    if (shm_name) shm_unlink(shm_name);
}

// KEYWORDS: IPC CLOSE
// Close common IPC handles (mq descriptor, semaphore, shared memory
// fd) safely if they appear to be open.
void close_ipc(mqd_t mq, sem_t *sem, int shm_fd) {
    if (mq != (mqd_t)-1) mq_close(mq);
    if (sem != SEM_FAILED && sem != NULL) sem_close(sem);
    if (shm_fd >= 0) close(shm_fd);
}

// KEYWORDS: PRODUCER_CONSUMER SEMAPHORE
// Template for a producer/consumer ring buffer using three
// semaphores. The ProducerConsumer struct holds semaphores for
// counting empty slots and filled slots and a mutex to protect the
// buffer while inserting/removing. The functions below are
// placeholders showing the correct semaphore order — they must be
// adapted to the specific buffer data structure used.
typedef struct {
    sem_t *empty;  // Counts empty slots
    sem_t *full;   // Counts full slots
    sem_t *mutex;  // Protects buffer
    void *buffer;  // Pointer to user-managed buffer
    size_t size;   // Size of buffer / number of slots
} ProducerConsumer;

// Add an item into the producer/consumer buffer. This template:
//  - waits for an empty slot
//  - locks the buffer
//  - inserts the item (user must implement copying index logic)
//  - unlocks and signals a full slot
void producer_add(ProducerConsumer *pc, void *item) {
    sem_wait(pc->empty);   // Wait for empty slot
    sem_wait(pc->mutex);   // Lock buffer
    
    // Add item to buffer (implementation-specific)
    
    sem_post(pc->mutex);   // Unlock buffer
    sem_post(pc->full);    // Signal item added
}

// Remove an item from the buffer. This template:
//  - waits for a full slot
//  - locks the buffer
//  - removes the item (user must implement copying/index logic)
//  - unlocks and signals an empty slot
void consumer_get(ProducerConsumer *pc, void *item) {
    sem_wait(pc->full);    // Wait for item
    sem_wait(pc->mutex);   // Lock buffer
    
    // Remove item from buffer (implementation-specific)
    
    sem_post(pc->mutex);   // Unlock buffer
    sem_post(pc->empty);   // Signal slot freed
}

// KEYWORDS: SENTINEL MQUEUE
// Send a sentinel message to each worker via the message queue. The
// sentinel is an agreed-upon byte pattern (often an empty filename
// or a special struct field) that tells a worker to stop.
void send_sentinels(mqd_t mq, int num_workers, void *sentinel, size_t size) {
    for (int i = 0; i < num_workers; i++) {
        mq_send(mq, (char *)sentinel, size, 0);
    }
}

// KEYWORDS: SENTINEL UTILS
// Check whether a received message matches the sentinel pattern.
// Plain-English: "Did the sender send the 'stop' message?"
int is_sentinel(void *msg, void *sentinel, size_t size) {
    return memcmp(msg, sentinel, size) == 0;
}

// KEYWORDS: SHM POINTER_OFFSETS
// Convert a pointer inside a shared memory mapping to an offset from
// the mapping base. Using offsets (instead of absolute pointers)
// allows different processes to share linked structures safely even
// if the mapping address differs between processes.
size_t ptr_to_offset(void *base, void *ptr) {
    if (ptr == NULL) return 0;
    return (char *)ptr - (char *)base;
}

// KEYWORDS: SHM POINTER_OFFSETS
// Convert an offset back to a pointer relative to the base mapping.
void *offset_to_ptr(void *base, size_t offset) {
    if (offset == 0) return NULL;
    return (char *)base + offset;
}

// Add node to front of list in shared memory
void shm_list_add(void *shm_base, size_t *head_offset, void *new_node, size_t node_size) {
    size_t new_offset = /* current end of shm */;
    
    // Copy new_node data to shm_base + new_offset
    memcpy((char *)shm_base + new_offset, new_node, node_size);
    
    // Update next pointer (stored as offset)
    *(size_t *)((char *)shm_base + new_offset + offsetof(YourStruct, next_offset)) = *head_offset;
    
    // Update head
    *head_offset = new_offset;
}

int main(int argc, char *argv[]) {
    // 1. Parse arguments
    
    // 2. Create IPC resources
    mqd_t mq = create_mqueue("/name", sizeof(Task));
    sem_t *sem = create_semaphore("/sem");
    int shm_fd;
    void *shm = create_shared_memory("/shm", 4096, &shm_fd);
    
    // 3. Initialize shared data
    
    // 4. Fork workers
    pid_t *pids = fork_workers(num_workers, "./worker", args);
    
    // 5. Send work
    for (/* each task */) {
        mq_send(mq, (char *)&task, sizeof(task), 0);
    }
    send_sentinels(mq, num_workers, &sentinel, sizeof(sentinel));
    
    // 6. Wait for workers
    wait_for_workers(pids, num_workers);
    free(pids);
    
    // 7. Process results
    
    // 8. Cleanup
    munmap(shm, size);
    close(shm_fd);
    sem_close(sem);
    mq_close(mq);
    cleanup_ipc("/name", "/sem", "/shm");
    
    return 0;
}

int main(int argc, char *argv[]) {
    // 1. Parse arguments
    
    // 2. Open IPC resources
    mqd_t mq = open_mqueue_read(queue_name);
    sem_t *sem = open_semaphore(sem_name);
    int shm_fd;
    void *shm = open_shared_memory(shm_name, size, &shm_fd);
    
    // 3. Main loop
    Task task;
    while (mq_receive(mq, (char *)&task, sizeof(task), NULL) > 0) {
        if (is_sentinel(&task, &sentinel, sizeof(task))) break;
        
        // Process task
        
        // Update shared memory
        sem_wait(sem);
        // ... critical section ...
        sem_post(sem);
    }
    
    // 4. Cleanup
    munmap(shm, size);
    close(shm_fd);
    sem_close(sem);
    mq_close(mq);
    
    return 0;
}

