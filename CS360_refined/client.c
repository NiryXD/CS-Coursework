#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#define Buffer 4096

int main(int argc, char *argv[])
{

    if (argc != 4) {
        fprintf(stderr, "Usage: %s ___ ____ ____\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        return 1;
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rc = getaddrinfo(host, port, &hints, &result);
if (rc != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
    close(socketFd);
    return 1;
}

    if (connect(socketFd, result->ai_addr, result->ai_addrlen) < 0) {
        freeaddrinfo(result);
        close(socketFd);
        return 1;
    }

    freeaddrinfo(result);

    char request[1024];
    snprintf(request, sizeof(request), 
    "GET %s HTTP/1.1\r\n" 
    "Host: %s\r\n"
    "Connection: close\r\n"
    "\r\n",
    path, host);

    if (send(socketFd, request, strlen(request), 0) < 0) {
        close(socketFd);
        return 1;
    }

    char buffer[Buffer];
    ssize_t bytes;
    int header = 0;
    char *start = NULL;

    char *respond = NULL;
    size_t rSize = 0;

    while ((bytes = recv(socketFd, buffer, Buffer, 0)) > 0) {
        respond = realloc(respond, rSize + bytes);
        memcpy(respond + rSize, buffer, bytes);
        rSize += bytes;

        if(!header) {
            for (size_t i = 0; i < rSize - 3; i++) {
                if (respond[i] == '\r' && respond[i+1] == '\n' && respond[i+2] == '\r' && respond[i+3] == '\n') {
                    header = 1;
                    start = respond + i + 4;
                    fwrite(start, 1, rSize - (size_t)(start - respond), stdout);
                    break;
                }
            }
        }
        else {
            fwrite(buffer, 1, bytes, stdout);
        }
    }

    if (respond) {
        free(respond);
    }
    close(socketFd);
    return 0;
            
}
