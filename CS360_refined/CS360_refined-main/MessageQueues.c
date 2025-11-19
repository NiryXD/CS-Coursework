#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>

// RECEIVER
struct Data {
    char   key[32];
    int value;
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <mqueue file>\n", argv[0]);
        return 1;
    }

    mqd_t mq;
    struct Data data;
    ssize_t bytes;
    unsigned int prio;

    mq = mq_open(argv[1], O_RDONLY);

    if (mq <0) {
        perror("mq_open");
        return 1;
    }

    for (;;) {
        bytes = mq_receive(mq, (char*)&data, sizeof(data), &prio);
        if (bytes < 0) {
            perror("mq_recieve");
            break;
        }
        else if (!strcmp(data.key, "QUIT")) {
            break;
        }
        printf("%d: %s\n", data.value, data.key);
    }

    mq_close(mq);
    return 0;
}
