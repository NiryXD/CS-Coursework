#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>

#include "hfactor.h"

ssize_t writeAll (int fd, const void *buffer, size_t totalBytes ) {
    const char *current = (const char *)buffer;
    size_t remain = totalBytes;
    while (remain > 0 ) {
        ssize_t wrote = write(fd, current, remain);
        if (wrote <= 0) return -1;
        current += wrote;
        remain  -= (size_t)wrote;
    }
    return (ssize_t)totalBytes;
}

ssize_t readAll (int fd, void *buffer, size_t totalBytes) {
    char *current = (char *)buffer;
    size_t remain = totalBytes;
    while (remain > 0) {
        ssize_t r = read(fd, current, remain);
        if (r <= 0) return -1;
        current += r;
        remain  -= (size_t)r;
    }
    return (ssize_t)totalBytes;
}

int hfactor_process(uint64_t target)
{
    mqd_t q = mq_open(QUEUE_NAME, O_RDONLY);
    if (q == (mqd_t)-1) {
        perror("mq_open (child)");
        return -1;
    }

    sem_t *sem = sem_open(SEMAPHORE_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open (child)");
        mq_close(q);
        return -1;
    }

    int shm = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm == -1) {
        perror("shm_open (child)");
        sem_close(sem);
        mq_close(q);
        return -1;
    }

    for (;;) {
        uint64_t candidate = 0;
        ssize_t n = mq_receive(q, (char*)&candidate, sizeof(candidate), NULL);
        if (n < 0) {
            perror("mq_receive");
            break;
        }
        if (candidate == 0) {
            break;
        }

        if (target % candidate == 0) {
            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                break;
            }

            uint64_t size = 0;
            if (lseek(shm, 0, SEEK_SET) < 0 ||
                read(shm, &size, sizeof(size)) != (ssize_t)sizeof(size)) {
                perror("read(size)");
                sem_post(sem);
                break;
            }

            off_t new_bytes = (off_t)(sizeof(uint64_t) + (size + 1) * sizeof(uint64_t));
            if (ftruncate(shm, new_bytes) == -1) {
                perror("ftruncate");
                sem_post(sem);
                break;
            }

            off_t slot = (off_t)(sizeof(uint64_t) + size * sizeof(uint64_t));
            if (lseek(shm, slot, SEEK_SET) < 0 ||
                write(shm, &candidate, sizeof(candidate)) != (ssize_t)sizeof(candidate)) {
                perror("write(factor)");
                sem_post(sem);
                break;
            }

            ++size;
            if (lseek(shm, 0, SEEK_SET) < 0 ||
                write(shm, &size, sizeof(size)) != (ssize_t)sizeof(size)) {
                perror("write(size)");
                sem_post(sem);
                break;
            }

            if (sem_post(sem) == -1) {
                perror("sem_post");
                break;
            }
        }
    }

    close(shm);
    sem_close(sem);
    mq_close(q);
    return 0;
}

int hfactor_init(mqd_t *q, sem_t **sem, int *shm)
{
    if (!q || !sem || !shm) return -1;

    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = sizeof(uint64_t);
    attr.mq_curmsgs = 0;

    *q = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
    if (*q == (mqd_t)-1) {
        perror("mq_open (init)");
        return -1;
    }

    *sem = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (*sem == SEM_FAILED) {
        perror("sem_open (init)");
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    *shm = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (*shm == -1) {
        perror("shm_open (init)");
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    uint64_t zero = 0;
    if (ftruncate(*shm, sizeof(zero)) == -1) {
        perror("ftruncate (init)");
        close(*shm);
        shm_unlink(SHM_NAME);
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }
    if (lseek(*shm, 0, SEEK_SET) < 0 ||
        write(*shm, &zero, sizeof(zero)) != (ssize_t)sizeof(zero)) {
        perror("write(size=0)");
        close(*shm);
        shm_unlink(SHM_NAME);
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    return 0;
}

void hfactor_deinit(mqd_t q, sem_t *sem, int shm)
{
    if (q != (mqd_t)-1) mq_close(q);
    if (sem && sem != SEM_FAILED) sem_close(sem);
    if (shm >= 0) close(shm);

    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);
}

int hfactor_report(const char *reporter, int shm)
{
    if (!reporter || shm < 0) return -1;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork (report)");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[0]);
        execl(reporter, reporter, (char *)NULL);
        perror("execl");
        exit(1);
    }

    close(pipefd[0]);

    uint64_t size = 0;
    if (lseek(shm, 0, SEEK_SET) < 0 ||
        read(shm, &size, sizeof(size)) != (ssize_t)sizeof(size)) {
        perror("read(size for report)");
        close(pipefd[1]);
        return -1;
    }

    if (write(pipefd[1], &size, sizeof(size)) != (ssize_t)sizeof(size)) {
        perror("write(size to pipe)");
        close(pipefd[1]);
        return -1;
    }

    for (uint64_t i = 0; i < size; ++i) {
        uint64_t v = 0;
        off_t off = (off_t)(sizeof(uint64_t) + i * sizeof(uint64_t));
        if (lseek(shm, off, SEEK_SET) < 0 ||
            read(shm, &v, sizeof(v)) != (ssize_t)sizeof(v) ||
            write(pipefd[1], &v, sizeof(v)) != (ssize_t)sizeof(v)) {
            perror("send factor to pipe");
            close(pipefd[1]);
            (void)waitpid(pid, NULL, 0);
            return -1;
        }
    }

    close(pipefd[1]);
    (void)waitpid(pid, NULL, 0);
    return 0;
}

int hfactor_calculate(const char *reporter, uint64_t target, uint64_t num_workers)
{
    mqd_t q = (mqd_t)-1;
    sem_t *sem = SEM_FAILED;
    int shm = -1;

    if (hfactor_init(&q, &sem, &shm) == -1) {
        return -1;
    }

    close(shm);
    shm = -1;
    sem_close(sem);
    sem = NULL;
    mq_close(q);
    q = (mqd_t)-1;

    pid_t *workers = (pid_t *)malloc(num_workers * sizeof(pid_t));
    if (!workers) {
        perror("malloc(workers)");
        q   = mq_open(QUEUE_NAME, O_RDWR);
        sem = sem_open(SEMAPHORE_NAME, 0);
        shm = shm_open(SHM_NAME, O_RDWR, 0666);
        hfactor_deinit(q, sem, shm);
        return -1;
    }

    for (uint64_t i = 0; i < num_workers; ++i) {
        workers[i] = fork();
        if (workers[i] == -1) {
            perror("fork (worker)");
            for (uint64_t j = 0; j < i; ++j) {
                if (workers[j] > 0) kill(workers[j], SIGTERM);
            }
            free(workers);
            q   = mq_open(QUEUE_NAME, O_RDWR);
            sem = sem_open(SEMAPHORE_NAME, 0);
            shm = shm_open(SHM_NAME, O_RDWR, 0666);
            hfactor_deinit(q, sem, shm);
            return -1;
        }
        if (workers[i] == 0) {
            int ret = hfactor_process(target);
            exit(ret == 0 ? 0 : 1);
        }
    }

    q = mq_open(QUEUE_NAME, O_WRONLY);
    if (q == (mqd_t)-1) {
        perror("mq_open (parent producer)");
        for (uint64_t i = 0; i < num_workers; ++i) {
            if (workers[i] > 0) kill(workers[i], SIGTERM);
        }
        free(workers);
        sem = sem_open(SEMAPHORE_NAME, 0);
        shm = shm_open(SHM_NAME, O_RDWR, 0666);
        hfactor_deinit(q, sem, shm);
        return -1;
    }

    for (uint64_t candidate = 2; candidate <= (target / 2); ++candidate) {
        if (mq_send(q, (const char*)&candidate, sizeof(candidate), 0) == -1) {
            perror("mq_send(candidate)");
            break;
        }
    }

    uint64_t sentinel = 0;
    for (uint64_t i = 0; i < num_workers; ++i) {
        if (mq_send(q, (const char*)&sentinel, sizeof(sentinel), 0) == -1) {
            perror("mq_send(sentinel)");
        }
    }

    mq_close(q);

    for (uint64_t i = 0; i < num_workers; ++i) {
        int st = 0;
        (void)waitpid(workers[i], &st, 0);
    }
    free(workers);

    shm = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm == -1) {
        perror("shm_open (report)");
        q   = mq_open(QUEUE_NAME, O_RDWR);
        sem = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(q, sem, shm);
        return -1;
    }

    if (hfactor_report(reporter, shm) == -1) {
        close(shm);
        q   = mq_open(QUEUE_NAME, O_RDWR);
        sem = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(q, sem, shm);
        return -1;
    }

    q   = mq_open(QUEUE_NAME, O_RDWR);
    sem = sem_open(SEMAPHORE_NAME, 0);
    hfactor_deinit(q, sem, shm);
    return 0;
}
