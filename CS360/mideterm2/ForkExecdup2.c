/*
 * ForkExecdup2.c
 *
 * Demonstrates forking a child process, redirecting its standard output
 * and standard error to files using dup2, and then running a different
 * program in the child via execv. The program includes layman-friendly
 * comments explaining key concepts: pid, fork, dup2, and exec.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    /* Expect three arguments: path to stdout file, path to stderr file,
     * and the command to run (with optional args following it).
     * Example: ./a.out out.log err.log /bin/ls -l /
     */
    if (argc < 4) {
        printf("Usage: %s <output> <error> <command> [args]\n", argv[0]);
        return 1;
    }

    /* Open the files that will capture the child's stdout and stderr.
     * O_TRUNC ensures existing files are cleared. These calls return
     * file descriptors (small integers) that refer to open files in the OS.
     */
    int fd1 = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    /* pid_t is the integer type used for process IDs (PIDs).
     * The variable `pid` will hold the result of fork():
     *  - In the parent, fork() returns the child's PID (a positive integer).
     *  - In the child, fork() returns 0.
     *  - On error, fork() returns -1.
     */
    pid_t pid;
    pid = fork();

    if (pid == 0){
        /* This is the child process (fork returned 0).
         * dup2(oldfd, newfd) makes newfd refer to the same open file as
         * oldfd. Here we redirect the child's standard output (file
         * descriptor 1) to fd1, and standard error (fd 2) to fd2.
         * After these calls, any printf() or writes to stdout/stderr in
         * the child will go into the specified files instead of the
         * terminal.
         */
        dup2(fd1, STDOUT_FILENO);
        dup2(fd2, STDERR_FILENO);

        /* execv replaces the current process image with a new program.
         * The child will become the program named by argv[3], using the
         * argument array starting at argv+3. If execv succeeds it does
         * not return — the new program runs in this process slot. If it
         * returns, an error occurred and the child should usually exit.
         */
        execv(argv[3], argv + 3);
        /* If execv fails, we fall through and the child will exit here.
         * In a robust program you would print an error message.
         */
    }
    else{
        /* Parent process: wait for the child to finish and collect its
         * exit status. waitpid(pid, &status, 0) waits specifically for
         * that child process (using the child's PID returned earlier).
         */
        int status;
        waitpid(pid, &status, 0);
        /* WIFEXITED(status) is true (nonzero) if the child exited
         * normally (rather than being killed by a signal). The original
         * program prints "Child failed." when WIFEXITED(status) != 0,
         * which is a confusing test: it treats normal exit as failure.
         * A clearer check would inspect WEXITSTATUS(status) for the
         * child's numeric exit code. We leave the original logic
         * intact here but note the potential confusion for learners.
         */
        if(WIFEXITED(status) != 0){
            printf("Child failed.\n");
            return 1;
        }
    }

    return 0;
}