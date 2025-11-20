#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (3 != argc) {
        fprintf(stderr, "Usage: %s <socket> <data>\n", argv[0]);
        return 1;
    }

    struct sockaddr_un sun = {0};
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, argv[1], sizeof(sun.sun_path) - 1);
    int sfd;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == sfd){
        perror("socket");
        return 1;
    }

    if (-1 == connect(sfd, (struct sockaddr *)&sun, sizeof(sun))){
        perror("connect");
        return 1;
    }

    ssize_t bytes;

    char buffer[256];
    bytes = send(sfd, argv[2], strlen(argv[2]), MSG_NOSIGNAL);

    bytes = recv(sfd, buffer, sizeof(buffer) - 1, MSG_NOSIGNAL);

    if (bytes == -1){
        perror("recv");
    } else {
        buffer[bytes] = '\0';
    }
    printf("%s\n", buffer);

    close(sfd);

    return 0;
}
