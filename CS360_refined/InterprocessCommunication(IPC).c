#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    double x;
    double y;
    double z;
} DATA;

static void fail(const char *msg) { perror(msg); exit(1); }

int main(int argc, char *argv[])
{
    if (argc < 4) {
        printf("Usage: %s <mqueue> <sem> <shm>\n", argv[0]);
        return 1;
    }

    const char *queue_name = argv[1];
    const char *sem_name   = argv[2];
    const char *mem_name   = argv[3];

    const DATA stop = { -1, -1, -1 };

    mqd_t queue = mq_open(queue_name, O_RDONLY);
    if (queue == (mqd_t)-1) fail("mq_open");

    sem_t *lock = sem_open(sem_name, 0);
    if (lock == SEM_FAILED) fail("sem_open");

    int mem_fd = shm_open(mem_name, O_RDWR, 0);
    if (mem_fd < 0) fail("shm_open");

    void *mem = mmap(NULL, sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    if (mem == MAP_FAILED) fail("mmap");
    double *sum = (double *)mem;

    struct mq_attr qinfo;
    if (mq_getattr(queue, &qinfo) == -1) fail("mq_getattr");
    if ((size_t)qinfo.mq_msgsize < sizeof(DATA)) {
        fprintf(stderr, "queue msgsize too small\n");
        return 1;
    }
    char *buf = malloc((size_t)qinfo.mq_msgsize);
    if (!buf) fail("malloc");

    for (;;) {
        unsigned int prio = 0;
        ssize_t n = mq_receive(queue, buf, (size_t)qinfo.mq_msgsize, &prio);
        if (n < 0) fail("mq_receive");

        DATA *msg = (DATA *)buf;
        if (msg->x == stop.x && msg->y == stop.y && msg->z == stop.z) break;

        double result = msg->x * msg->y + msg->z;

        if (sem_wait(lock) == -1) fail("sem_wait");
        double now = *sum;
        now += result;
        *sum = now;
        if (sem_post(lock) == -1) fail("sem_post");
    }

    free(buf);
    munmap(mem, sizeof(double));
    close(mem_fd);
    sem_close(lock);
    mq_close(queue);
    return 0;
}
