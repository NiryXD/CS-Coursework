#/*******************************************************
 * SharedMemory.c
 *
 * Simple driver program that demonstrates creating a named semaphore
 * and a named shared-memory object, initializing an integer counter,
 * then forking N worker processes that `execl` a provided worker
 * program. The workers are expected to open the same semaphore and
 * shared-memory object and update the counter in a coordinated way.
 *
 * This file contains plain-language comments to explain what each
 * major step does so newcomers to inter-process communication (IPC)
 * can follow along.
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>  /* POSIX named semaphores */
#include <sys/mman.h>   /* POSIX shared memory (shm_open/mmap) */
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    /* Variables used by the program */
    int jog;      /* forwarded numeric parameter for the worker program */
    int num;      /* number of worker processes to spawn */
    int shm;      /* file descriptor for the shared-memory object */
    int i;        /* general-purpose loop/index */
    sem_t *sem;   /* pointer to a named semaphore object */
    
    /* Expect five arguments: semaphore name, shared-memory name,
     * number of workers, a numeric 'jog' parameter for the worker,
     * and the path to the worker executable.
     */
    if (argc < 6) {
        printf("Usage: %s <sem file> <shm file> <num> <jog> <worker>\n", argv[0]);
        return 1;
    }
    if (1 != sscanf(argv[3], "%d", &num) || num <= 0 || num > 100000) {
        printf("Invalid number for num '%s'\n", argv[3]);
        return 1;
    }
    if (1 != sscanf(argv[4], "%d", &jog) || jog <= 0 || jog > 100000) {
        printf("Invalid number for jog '%s'\n", argv[4]);
        return 1;
    }

    /* Create a named semaphore with initial value 1 (acts like a mutex).
     * O_CREAT | O_EXCL means "create only if it doesn't already exist";
     * that prevents accidentally reusing an existing semaphore name.
     *
     * Note: on error POSIX `sem_open` returns SEM_FAILED (not NULL). The
     * code below checks for NULL as in the original program; a more
     * portable check would be `if (sem == SEM_FAILED)`.
     */
    sem = sem_open(argv[1], O_CREAT | O_EXCL | O_RDWR, 0600, 1);
    if (sem == NULL) {
        perror("sem_open");
        return 1;
    }

    /* Create a named shared-memory object and set its size to 4 bytes
     * (enough to hold a 32-bit integer). The shared object is a kernel
     * resource that multiple processes can open and read/write by name.
     *
     * The program writes an initial value of 0 into the shared object
     * so workers start from a known counter value.
     */
    shm = shm_open(argv[2], O_CREAT | O_RDWR | O_EXCL, 0600);
    if (shm == -1) {
        perror("shm_open");
        return 1;
    }
    /* Set the size of the shared memory region (4 bytes for an int). */
    ftruncate(shm, 4);
    i = 0;
    /* Write the initial integer value (0) into the shared-memory object
     * via the file descriptor. Another common approach is to `mmap`
     * the object and store the value directly in memory; here the FD
     * write is a simple initialization step.
     */
    write(shm, &i, sizeof(i));

    /* Fork `num` worker processes. Each child immediately replaces itself
     * with the provided worker executable using `execl`. The worker will
     * receive the semaphore name and shared-memory name as its first two
     * arguments, plus `num` and `jog`. This allows the worker to open
     * the same named semaphore/shm and perform coordinated updates.
     *
     * Note: the parent stores each child's pid so it can wait for them.
     */
    pid_t *pids = malloc(sizeof(pid_t) * num);
    for (i = 0; i < num; i += 1) {
        pids[i] = fork();
        if (pids[i] == 0) {
            /* In the child: close the shared-memory FD (the worker is
             * expected to open by name if it needs to). Then build the
             * argument strings and exec the worker program.
             */
            close(shm);
            char value[16];
            char sjog[16];
            sprintf(value, "%d", num);
            sprintf(sjog, "%d", jog);
            execl(argv[5], argv[5], argv[1], argv[2], value, sjog, NULL);
            /* If execl returns, an error occurred. Print and exit child. */
            perror("execl");
            return 1;
        }
    }

    /* Parent: wait for all children to finish their work. */
    for (i = 0; i < num; i += 1) {
        waitpid(pids[i], NULL, 0);
    }

    /* Cleanup: close the shared-memory FD and semaphore. The unlink
     * calls are commented out in the original program; uncommenting them
     * would remove the named objects from the system so subsequent runs
     * can recreate them with the same names.
     */
    close (shm);
    //shm_unlink(argv[2]); /* remove named shared memory from system */
    free(pids);
    sem_close(sem);
    //sem_unlink(argv[1]); /* remove named semaphore from system */


    return 0;
}