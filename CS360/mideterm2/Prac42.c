#include "psearch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>

#define NUM_THREADS 4

/*
 * Prac42.c - multi-threaded searcher (worker)
 *
 * This program is intended to be run as a worker process by the
 * parallel search coordinator. It listens on a POSIX message queue
 * for SearchTask messages (each containing a filename). For each file
 * task it receives it:
 *  1) reads the entire file into memory
 *  2) spawns NUM_THREADS threads to search different sections of the
 *     file in parallel for the requested pattern
 *  3) collects up to 100 matching line numbers into a SearchResult
 *  4) writes the SearchResult back to the parent via a pipe (fd
 *     passed on the command line)
 *
 * The comments in this file explain each step in plain English and
 * clarify why certain synchronization (mutexes) and data layout
 * choices are made.
 */

// Shared data for threads. Each thread gets a ThreadData describing
// what part of the file to search and where to store matches.
typedef struct {
    char *file_contents;      // pointer to the whole file buffer
    size_t file_size;         // total bytes in file
    char pattern[256];        // search pattern (copied per thread)
    size_t pattern_len;      
    int start_offset;         // byte index where this thread starts
    int end_offset;           // byte index where this thread ends
    int *line_numbers;        // shared array for storing matching line numbers
    int *match_count;         // shared counter of matches found
    int *line_count;          // optional shared line number tracker (not heavily used)
    pthread_mutex_t *mutex;   // mutex to protect shared match_count/line_numbers
} ThreadData;

/*
 * search_section
 * This is the function each thread runs. It searches only the bytes
 * from start_offset to end_offset so threads operate on disjoint
 * ranges (except when matches cross a boundary — a simple tradeoff
 * given the course-level example). For each found occurrence the
 * thread computes the line number by scanning from the file start to
 * the match position (this is simple but not the most efficient
 * approach). When a match is found the thread locks the mutex to
 * update the shared results array and counter.
 */
void *search_section(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    if (data->start_offset >= data->end_offset) {
        return NULL; // empty region
    }

    char *start = data->file_contents + data->start_offset;
    char *end = data->file_contents + data->end_offset;

    /*
     * Compute the line number at the start of our section by counting
     * newline characters from the beginning of the file up to start.
     * This lets us report the line number for matches inside this
     * thread without needing global line-indexing.
     */
    int line = 1;
    for (char *p = data->file_contents; p < start; p++) {
        if (*p == '\n') line++;
    }

    char *pos = start;

    while (pos < end) {
        // Find next occurrence of the pattern starting at pos
        char *match = strstr(pos, data->pattern);

        // If no match was found or the match is beyond our section, stop
        if (match == NULL || match >= end) {
            break;
        }

        // Advance pos to the match while keeping track of line numbers
        while (pos < match) {
            if (*pos == '\n') line++;
            pos++;
        }

        /*
         * Protect shared state (match_count and line_numbers) with a
         * mutex. We only store up to 100 line numbers to keep the
         * result struct bounded and avoid unbounded memory use.
         */
        pthread_mutex_lock(data->mutex);

        if (*data->match_count < 100) {
            data->line_numbers[*data->match_count] = line;
        }
        (*data->match_count)++;

        pthread_mutex_unlock(data->mutex);

        // Move past this match so we don't find the same occurrence again
        pos = match + data->pattern_len;
    }

    return NULL;
}

/*
 * read_file
 * Convenience helper that reads an entire file into a malloc'd
 * buffer and returns its size. The buffer is NUL-terminated to make
 * string operations easier. The caller must free the buffer.
 */
char *read_file(const char *filename, size_t *size) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror(filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < 0) {
        fclose(f);
        return NULL;
    }

    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, f);
    buffer[bytes_read] = '\0';
    *size = bytes_read;

    fclose(f);
    return buffer;
}

/*
 * main
 * Arguments expected by the coordinator when it execs this worker:
 *   argv[1] = mqueue name (to receive SearchTask messages)
 *   argv[2] = pattern to search for
 *   argv[3] = pipe_fd (integer file descriptor where results are written)
 *   argv[4] = num_workers (not used heavily here but passed through)
 */
int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Searcher usage: %s <mqueue> <pattern> <pipe_fd> <num_workers>\n", argv[0]);
        return 1;
    }

    const char *queue_name = argv[1];
    const char *pattern = argv[2];
    int pipe_fd = atoi(argv[3]);
    size_t pattern_len = strlen(pattern);

    /*
     * Open the named message queue for reading tasks. The parent
     * created this queue and will send SearchTask messages.
     */
    mqd_t mq = mq_open(queue_name, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("searcher: mq_open");
        return 1;
    }

    // Main loop: process incoming SearchTask messages
    SearchTask task;

    while (1) {
        ssize_t received = mq_receive(mq, (char *)&task, sizeof(task), NULL);
        if (received == -1) {
            perror("searcher: mq_receive");
            break;
        }

        // A sentinel message uses an empty filename to indicate "no more work"
        if (task.filename[0] == '\0') {
            break;
        }

        // Read the entire file so threads can search different parts
        size_t file_size;
        char *file_contents = read_file(task.filename, &file_size);

        if (!file_contents) {
            /* If the file can't be read, send an empty result back so
             * the parent knows this file produced no matches (or was
             * unreadable). This keeps the protocol simple.
             */
            SearchResult result;
            memset(&result, 0, sizeof(result));
            strncpy(result.filename, task.filename, sizeof(result.filename) - 1);
            result.match_count = 0;
            result.num_lines = 0;
            write(pipe_fd, &result, sizeof(result));
            continue;
        }

        /*
         * Prepare thread-shared buffers: a fixed-size array for line
         * numbers (we cap at 100), a match counter, and a mutex to
         * protect concurrent updates. This keeps the result bounded
         * in size and avoids dynamic resizing while threads run.
         */
        int line_numbers[100];
        int match_count = 0;
        int line_count = 0;
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

        // Create threads to search different sections in parallel
        pthread_t threads[NUM_THREADS];
        ThreadData thread_data[NUM_THREADS];

        /* Divide the file into NUM_THREADS roughly equal sections by
         * byte range. Each thread gets a start and end offset.
         */
        size_t section_size = (file_size + NUM_THREADS - 1) / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            thread_data[i].file_contents = file_contents;
            thread_data[i].file_size = file_size;
            strncpy(thread_data[i].pattern, pattern, sizeof(thread_data[i].pattern) - 1);
            thread_data[i].pattern_len = pattern_len;
            thread_data[i].start_offset = i * section_size;
            thread_data[i].end_offset = (i + 1) * section_size;
            if (thread_data[i].end_offset > file_size) {
                thread_data[i].end_offset = file_size;
            }
            thread_data[i].line_numbers = line_numbers;
            thread_data[i].match_count = &match_count;
            thread_data[i].line_count = &line_count;
            thread_data[i].mutex = &mutex;

            if (pthread_create(&threads[i], NULL, search_section, &thread_data[i]) != 0) {
                perror("pthread_create");
            }
        }

        // Wait for all threads to finish their work
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        pthread_mutex_destroy(&mutex);

        // Prepare a SearchResult to send back to the parent via the pipe
        SearchResult result;
        memset(&result, 0, sizeof(result));
        strncpy(result.filename, task.filename, sizeof(result.filename) - 1);
        result.match_count = match_count;
        result.num_lines = (match_count < 100) ? match_count : 100;

        // Copy up to the first num_lines line numbers into the result
        for (int i = 0; i < result.num_lines; i++) {
            result.line_numbers[i] = line_numbers[i];
        }

        // Send the result struct back to the parent via the pipe
        if (write(pipe_fd, &result, sizeof(result)) != sizeof(result)) {
            perror("searcher: write to pipe");
        }

        free(file_contents);
    }

    // Cleanup: close the pipe and the message queue descriptor
    close(pipe_fd);
    mq_close(mq);

    return 0;
}