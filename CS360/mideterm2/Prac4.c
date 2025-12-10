/*
 * Prac4.c - parallel search coordinator
 *
 * This program demonstrates a simple master/worker pattern using POSIX
 * message queues for distributing work and a pipe for collecting
 * results. The parent (this program) does the following:
 *  1) creates a unique message queue name and a pipe for results
 *  2) forks several worker processes that exec the './searcher'
 *     program. Workers read tasks from the message queue and write
 *     their SearchResult back to the parent via the pipe
 *  3) parent sends each filename as a SearchTask in the queue and
 *     then sends one sentinel (empty filename) per worker so each
 *     worker knows when to stop
 *  4) parent reads SearchResult structs from the pipe and prints
 *     a summary once workers finish
 *
 * The comments below explain each step in plain language and point
 * out the important system calls (mq_open/mq_send, pipe, fork,
 * execl, read/write, waitpid, mq_unlink).
 */

#include "psearch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <num_workers> <pattern> <file1> [file2...]\n", argv[0]);
        return 1;
    }

    int num_workers = atoi(argv[1]);
    if (num_workers <= 0 || num_workers > 50) {
        fprintf(stderr, "Invalid number of workers: %d\n", num_workers);
        return 1;
    }

    const char *pattern = argv[2];
    int num_files = argc - 3;

    /* Create a unique, named POSIX message queue so workers can
     * receive tasks. Using the parent's PID in the name avoids clashes
     * with other runs. mq_unlink is called first to remove any stale
     * queue left behind by a crashed previous run.
     */
    char queue_name[256];
    snprintf(queue_name, sizeof(queue_name), "/psearch_%d", getpid());

    /* Remove any existing queue with the same name so we start fresh */
    mq_unlink(queue_name);

    /* Create the message queue. We set mq_msgsize to hold a
     * SearchTask structure. The parent will only write to this queue
     * (workers will open it for reading).
     */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(SearchTask);
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(queue_name, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    /* Create a unidirectional pipe for collecting results from
     * workers. Each worker will write its SearchResult to the write
     * end; the parent will read from the read end. Using a pipe is
     * a simple way to transfer fixed-size structs between processes.
     */
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        mq_close(mq);
        mq_unlink(queue_name);
        return 1;
    }

    /* Fork worker processes. We save the PIDs so we can wait for them
     * later. Each worker will exec the './searcher' binary which
     * connects to the named message queue and the pipe write-end.
     */
    pid_t *pids = malloc(sizeof(pid_t) * num_workers);
    if (!pids) {
        perror("malloc");
        close(pipefd[0]);
        close(pipefd[1]);
        mq_close(mq);
        mq_unlink(queue_name);
        return 1;
    }

    for (int i = 0; i < num_workers; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            // Kill already-forked workers
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            close(pipefd[0]);
            close(pipefd[1]);
            mq_close(mq);
            mq_unlink(queue_name);
            return 1;
        }

        if (pids[i] == 0) {
            /* Child process: close the read end (we only write results),
             * convert file descriptors/ints to strings, and use execl to
             * replace the child with the 'searcher' program. execl
             * does not return on success; if it returns an error we
             * print it and exit.
             */
            close(pipefd[0]);  // Close parent's read end in child

            /* Pass the write-end of the pipe and the number of workers
             * to the searcher as command-line arguments so the searcher
             * knows where to write results and how many sentinels to
             * expect if it needs that info.
             */
            char pipe_str[32];
            char workers_str[32];
            sprintf(pipe_str, "%d", pipefd[1]);
            sprintf(workers_str, "%d", num_workers);

            execl("./searcher", "searcher", queue_name, pattern, pipe_str, workers_str, NULL);
            perror("execl");
            exit(1);
        }
    }

    /* Parent closes its copy of the pipe write end because it will
     * only read results. Workers keep the write end open and send
     * SearchResult structs through it.
     */
    close(pipefd[1]);

    /* Send each filename as a SearchTask into the message queue. Each
     * worker (which opened the same queue name for reading) will take
     * tasks off the queue and perform the search on the specified
     * file. file_index is a small integer index used to identify the
     * file in results.
     */
    for (int i = 3; i < argc; i++) {
        SearchTask task;
        memset(&task, 0, sizeof(task));
        strncpy(task.filename, argv[i], sizeof(task.filename) - 1);
        task.file_index = i - 3;

        if (mq_send(mq, (char *)&task, sizeof(task), 0) == -1) {
            perror("mq_send");
        }
    }

    /* After sending all real tasks, send one sentinel message per
     * worker. A sentinel is an empty filename; workers interpret it
     * as "no more work" and exit. We then close the queue from the
     * parent's side (workers may still have it open).
     */
    SearchTask sentinel;
    memset(&sentinel, 0, sizeof(sentinel));
    sentinel.filename[0] = '\0';

    for (int i = 0; i < num_workers; i++) {
        if (mq_send(mq, (char *)&sentinel, sizeof(sentinel), 0) == -1) {
            perror("mq_send sentinel");
        }
    }

    mq_close(mq);

    /* Read SearchResult structs from the pipe. The parent prints a
     * header showing the search pattern, allocates a results array to
     * collect incoming structs, and reads until the pipe is closed by
     * all writers (workers). Each read returns sizeof(SearchResult)
     * bytes when a full struct was sent.
     */
    printf("\n=== Search Results for \"%s\" ===\n\n", pattern);

    SearchResult *results = malloc(sizeof(SearchResult) * num_files);
    if (!results) {
        perror("malloc results");
        free(pids);
        close(pipefd[0]);
        mq_unlink(queue_name);
        return 1;
    }

    int results_received = 0;
    SearchResult result;

    /* read() returns 0 when the write end is closed and all data is
     * consumed. We loop while we successfully read full SearchResult
     * structs and store them for printing later.
     */
    while (read(pipefd[0], &result, sizeof(result)) == sizeof(result)) {
        if (results_received < num_files) {
            results[results_received] = result;
            results_received++;
        }
    }

    close(pipefd[0]);

    /* Wait for each worker to exit. This ensures we don't leave
     * zombie processes behind. We don't currently inspect exit codes
     * closely here, but waitpid synchronizes the parent with child
     * termination.
     */
    for (int i = 0; i < num_workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    free(pids);

    /* Print a human-readable summary of the results we collected. For
     * each file we print the number of matches and up to the first 10
     * matching line numbers. We also keep a running total.
     */
    int total_matches = 0;

    for (int i = 0; i < results_received; i++) {
        if (results[i].match_count > 0) {
            printf("%s: %d match%s\n", 
                   results[i].filename, 
                   results[i].match_count,
                   results[i].match_count == 1 ? "" : "es");

            for (int j = 0; j < results[i].num_lines && j < 10; j++) {
                printf("  Line %d\n", results[i].line_numbers[j]);
            }

            if (results[i].num_lines > 10) {
                printf("  ... and %d more\n", results[i].num_lines - 10);
            }

            printf("\n");
            total_matches += results[i].match_count;
        } else {
            printf("%s: no matches\n\n", results[i].filename);
        }
    }

    printf("Total: %d matches across %d files\n", total_matches, num_files);

    free(results);

    /* Remove the named message queue from the system. This prevents
     * leftover named queues from accumulating if the program is run
     * repeatedly. Note: mq_unlink only removes the name; any open
     * descriptors are still valid until closed.
     */
    mq_unlink(queue_name);

    return 0;
}