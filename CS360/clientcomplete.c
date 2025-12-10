#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         // close
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>          // getaddrinfo, freeaddrinfo

#define BUFFER 4096

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <hostname> <port> <path>\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];

    // Resolve host/port
    struct addrinfo hints, *result = NULL, *rp = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    int rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 1;
    }

    // Try each addr until we connect
    int socketFd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socketFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketFd < 0) continue;

        if (connect(socketFd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break; // success
        }
        close(socketFd);
        socketFd = -1;
    }
    if (rp == NULL) {
        fprintf(stderr, "Could not connect to %s:%s\n", host, port);
        freeaddrinfo(result);
        return 1;
    }
    freeaddrinfo(result);

    // Build HTTP/1.1 request
    char request[1024];
    int n = snprintf(request, sizeof(request),
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     path, host);
    if (n < 0 || (size_t)n >= sizeof(request)) {
        fprintf(stderr, "Request too long\n");
        close(socketFd);
        return 1;
    }

    // Send it
    ssize_t sent = send(socketFd, request, (size_t)n, 0);
    if (sent < 0) {
        perror("send");
        close(socketFd);
        return 1;
    }

    // Read response; print only the body (after \r\n\r\n)
    char buffer[BUFFER];
    ssize_t bytes;
    int header_done = 0;

    // We’ll accumulate until we see the header/body boundary
    char *acc = NULL;
    size_t acc_size = 0;

    while ((bytes = recv(socketFd, buffer, sizeof(buffer), 0)) > 0) {
        if (!header_done) {
            // grow accumulator
            char *tmp = realloc(acc, acc_size + (size_t)bytes);
            if (!tmp) {
                fprintf(stderr, "Out of memory\n");
                free(acc);
                close(socketFd);
                return 1;
            }
            acc = tmp;
            memcpy(acc + acc_size, buffer, (size_t)bytes);
            acc_size += (size_t)bytes;

            // search for CRLF CRLF
            if (acc_size >= 4) {
                for (size_t i = 0; i + 3 < acc_size; i++) {
                    if (acc[i] == '\r' && acc[i+1] == '\n' &&
                        acc[i+2] == '\r' && acc[i+3] == '\n') {
                        header_done = 1;
                        size_t body_off = i + 4;
                        size_t body_len = acc_size - body_off;
                        if (body_len > 0) {
                            fwrite(acc + body_off, 1, body_len, stdout);
                        }
                        // from now on, print chunks directly
                        free(acc);
                        acc = NULL;
                        acc_size = 0;
                        break;
                    }
                }
            }
        } else {
            // header already done; write directly
            fwrite(buffer, 1, (size_t)bytes, stdout);
        }
    }

    if (acc) free(acc);
    close(socketFd);
    return 0;
}
