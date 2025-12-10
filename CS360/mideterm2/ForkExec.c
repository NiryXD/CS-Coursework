/*
 * ForkExec.c
 *
 * Simple program showing how to fork a child process and replace it
 * with a new program using execv. Includes plain-language comments
 * about the meaning of `pid` and the steps involved.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    /* pid_t is the integer type used for process IDs (PIDs) on POSIX.
     * The variable `pid` will hold the result of fork():
     *  - In the parent process, fork() returns the child's PID (a
     *    positive integer) so the parent knows which child it created.
     *  - In the child process, fork() returns 0, so the child can
     *    detect "I'm the child" and run child-only code.
     *  - On error, fork() returns -1 in the parent and no child is
     *    created.
     */
    pid_t pid;

    if (argc < 2) {
        printf("Usage: %s <command> [args]\n", argv[0]);
        return 1;
    }

    /* create a new process. After this call there are two running
     * processes: the original (parent) and the new one (child).
     */
    pid = fork();

    if (pid == 0) {
        /* This block runs only inside the child process because fork
         * returned 0 here. The child replaces its program image with
         * the program specified by argv[1] using execv. If execv
         * succeeds, it does not return; the child becomes the new
         * program. If execv returns, an error occurred.
         */
        execv(argv[1], argv + 1);
        perror("execv");
        return 111; /* return a distinctive code on exec failure */
    }

    /* Back in the parent process: wait for the child to finish and
     * collect its exit status. The parent uses the child's PID (the
     * positive value returned by fork) to wait specifically for that
     * child.
     */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        /* WIFEXITED tells us the child exited normally (not killed by a
         * signal). WEXITSTATUS extracts the child's exit code so the
         * parent can report it.
         */
        fprintf(stderr, "Child returned %d\n", WEXITSTATUS(status));
    }
    return 0;
}