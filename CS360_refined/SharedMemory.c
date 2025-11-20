#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int jog;
    int num;
    int shm;
    int i;
    sem_t *sem;
    
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

    sem = sem_open(argv[1], O_CREAT | O_EXCL | O_RDWR, 0600, 1);
    if (sem == NULL) {
        perror("sem_open");
        return 1;
    }

    shm = shm_open(argv[2], O_CREAT | O_RDWR | O_EXCL, 0600);
    if (shm == -1) {
        perror("shm_open");
        return 1;
    }
    ftruncate(shm, 4);
    i = 0;
    write(shm, &i, sizeof(i));

    pid_t *pids = malloc(sizeof(pid_t) * num);
    for (i = 0; i < num; i += 1) {
        pids[i] = fork();
        if (pids[i] == 0) {
            close(shm);
            char value[16];
            char sjog[16];
            sprintf(value, "%d", num);
            sprintf(sjog, "%d", jog);
            execl(argv[5], argv[5], argv[1], argv[2], value, sjog, NULL);
            perror("execl");
            return 1;
        }
    }

    for (i = 0; i < num; i += 1) {
        waitpid(pids[i], NULL, 0);
    }

    close (shm);
    //shm_unlink(argv[2]);
    free(pids);
    sem_close(sem);
    //sem_unlink(argv[1]);


    return 0;
}
