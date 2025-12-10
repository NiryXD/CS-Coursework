// client.c - COSC360 Sockets Lab: HTTP/1.1 client (IPv4, TCP)
// Usage: ./client <host> <port> <path>
// Prints ONLY the HTTP response body to stdout (no headers).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define REQ_MAX 4096
#define HDRBUF_MAX (128 * 1024)
#define RECV_CHUNK 4096

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <host> <port> <path>\n", argv[0]);
        return 1;
    }
    const char *host = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];

    // Resolve host (IPv4, TCP)
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 per lab
    hints.ai_socktype = SOCK_STREAM; // TCP

    struct addrinfo *res = NULL;
    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 2;
    }

    int sock = -1;
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) continue;
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
            break; // connected
        }
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    if (sock < 0) {
        die("connect");
    }

    // Build HTTP/1.1 request with CRLF line endings and blank line after headers
    char req[REQ_MAX];
    int n = snprintf(req, sizeof(req),
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     path, host);
    if (n < 0 || n >= (int)sizeof(req)) {
        fprintf(stderr, "Request too long\n");
        close(sock);
        return 3;
    }

    // Send the request (handle partial sends)
    const char *wptr = req;
    size_t wleft = (size_t)n;
    while (wleft > 0) {
        ssize_t w = send(sock, wptr, wleft, 0);
        if (w < 0) {
            if (errno == EINTR) continue;
            die("send");
        }
        wptr += (size_t)w;
        wleft -= (size_t)w;
    }

    // Read response: discard headers (until CRLF CRLF), print the body exactly
    char *hdrbuf = (char *)malloc(HDRBUF_MAX);
    if (!hdrbuf) die("malloc");
    size_t hdrlen = 0;
    int headers_done = 0;

    char buf[RECV_CHUNK];
    for (;;) {
        ssize_t r = recv(sock, buf, sizeof(buf), 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            die("recv");
        }
        if (r == 0) break; // EOF

        if (!headers_done) {
            // Accumulate into header buffer until we find "\r\n\r\n"
            if (hdrlen + (size_t)r > HDRBUF_MAX) {
                fprintf(stderr, "Headers too large\n");
                free(hdrbuf);
                close(sock);
                return 4;
            }
            memcpy(hdrbuf + hdrlen, buf, (size_t)r);
            hdrlen += (size_t)r;

            // Search for CRLF CRLF in hdrbuf
            size_t i;
            for (i = 3; i < hdrlen; i++) {
                if (hdrbuf[i-3] == '\r' && hdrbuf[i-2] == '\n' &&
                    hdrbuf[i-1] == '\r' && hdrbuf[i]   == '\n') {
                    size_t body_start = i + 1;
                    if (hdrlen > body_start) {
                        size_t body_bytes = hdrlen - body_start;
                        ssize_t wr = write(STDOUT_FILENO, hdrbuf + body_start, body_bytes);
                        (void)wr;
                    }
                    headers_done = 1;
                    break;
                }
            }
            continue;
        } else {
            ssize_t wr = write(STDOUT_FILENO, buf, (size_t)r);
            (void)wr;
        }
    }

    free(hdrbuf);
    close(sock);
    return 0;
}
