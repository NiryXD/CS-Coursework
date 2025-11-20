#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

static char *update(char data[]);

int main(int argc, char *argv[])
{
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <socket>\n", argv[0]);
        return 1;
    }

    struct sockaddr_un sun = {0};
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;
    int srvsock;
    int clnsock;
    char data[128];
    ssize_t bytes;

    srvsock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == srvsock) {
        perror("socket");
        return 1;
    }

    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, argv[1], sizeof(sun.sun_path) - 1);

    if (-1 == bind(srvsock, (struct sockaddr *)&sun, sizeof(sun))) {
        perror("bind");
        return 1;
    }

    listen(srvsock, 5);
    client_addr_len = sizeof(client_addr);
    clnsock = accept(srvsock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (-1 == clnsock) {
        perror("accept");
        goto out;
    }

    bytes = recv(clnsock, data, sizeof(data) - 1, MSG_NOSIGNAL);
    if (-1 == bytes) {
        perror("recv");
        goto out;
    }

    data[bytes] = '\0';

    bytes = send(clnsock, update(data), strlen(data), MSG_NOSIGNAL);
    if (-1 == bytes) {
        perror("send");
        goto out;
    }

out:
    close(srvsock);
    unlink(argv[1]);
    return 0;
}

static char *update(char data[])
{
    const int len = strlen(data);
    int i;
    for (i = 0; i < len; i++) {
        if (data[i] >= 'e') {
            data[i] = toupper(data[i]);
        }
    }
    return data;
}
