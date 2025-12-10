// server.c - COSC360 Sockets Lab: UNIX domain server with simple DNS lookup protocol
// Usage: ./server <unix_socket_path>
//
// Behavior per lab writeup:
//  - Create AF_UNIX, SOCK_STREAM server at given path, listen(backlog=5).
//  - Accept multiple simultaneous clients (non-blocking).
//  - On accept: send "HELLO" (5 bytes). Expect "HELLO" back within 1 second or disconnect.
//  - After handshake, accept commands:
//      * "BYE"                  -> close client
//      * "A\t<name>"            -> IPv4 lookup of <name>, reply "OK\t<addr>" or "NOTFOUND"
//      * "AAAA\t<name>"         -> IPv6 lookup of <name>, reply "OK\t<addr>" or "NOTFOUND"
//  - On SIGINT: gracefully close all clients, unlink socket file, exit.
//
// Notes:
//  - Sockets are non-blocking; main loop polls and nanosleeps for 100,000 ns if idle.
//  - For simplicity, commands are parsed from the beginning of the buffer. We accept both
//    newline-terminated messages or exact token messages (no newline).

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_CLIENTS 128
#define INBUF_SIZE 4096
#define HANDSHAKE_BYTES 5
#define HANDSHAKE_STR "HELLO"
#define NSLEEP_NS 100000L  // 100,000 nanoseconds

static volatile sig_atomic_t running = 1;
static char socket_path[108] = {0};

static void on_sigint(int signo) {
    (void)signo;
    running = 0;
}

static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

typedef struct {
    int fd;
    int handshake_done;
    struct timespec accepted_at;
    char inbuf[INBUF_SIZE];
    size_t inlen;
} Client;

static double seconds_since(const struct timespec *then) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double s = (double)(now.tv_sec - then->tv_sec);
    double ns = (double)(now.tv_nsec - then->tv_nsec) / 1e9;
    return s + ns;
}

static void client_init(Client *c) {
    c->fd = -1;
    c->handshake_done = 0;
    c->inlen = 0;
    c->accepted_at.tv_sec = 0;
    c->accepted_at.tv_nsec = 0;
}

static void client_close(Client *c) {
    if (c->fd >= 0) {
        close(c->fd);
    }
    client_init(c);
}

static int reply_notfound(int fd) {
    const char *msg = "NOTFOUND";
    ssize_t w = send(fd, msg, strlen(msg), 0);
    (void)w;
    return 0;
}

static int reply_ok_addr(int fd, const char *addrstr) {
    char msg[256];
    int n = snprintf(msg, sizeof(msg), "OK\t%s", addrstr);
    if (n < 0) return -1;
    ssize_t w = send(fd, msg, (size_t)n, 0);
    (void)w;
    return 0;
}

static void handle_lookup(int fd, int family, const char *name_raw, size_t name_len) {
    // Trim trailing CR/LF/space
    char name[1024];
    if (name_len >= sizeof(name)) name_len = sizeof(name) - 1;
    memcpy(name, name_raw, name_len);
    // Trim trailing whitespace
    while (name_len > 0 && (name[name_len-1] == '\r' || name[name_len-1] == '\n' || name[name_len-1] == ' ' || name[name_len-1] == '\t')) {
        name_len--;
    }
    name[name_len] = '\0';
    if (name_len == 0) {
        reply_notfound(fd);
        return;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family; // AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res = NULL;
    int rc = getaddrinfo(name, NULL, &hints, &res);
    if (rc != 0 || !res) {
        reply_notfound(fd);
        if (res) freeaddrinfo(res);
        return;
    }

    char addrbuf[INET6_ADDRSTRLEN];
    void *addrptr = NULL;
    if (res->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)res->ai_addr;
        addrptr = &(sa->sin_addr);
    } else if (res->ai_family == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)res->ai_addr;
        addrptr = &(sa6->sin6_addr);
    } else {
        freeaddrinfo(res);
        reply_notfound(fd);
        return;
    }

    if (!inet_ntop(res->ai_family, addrptr, addrbuf, sizeof(addrbuf))) {
        freeaddrinfo(res);
        reply_notfound(fd);
        return;
    }
    freeaddrinfo(res);
    reply_ok_addr(fd, addrbuf);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <socket file>\n", argv[0]);
        return 1;
    }
    // Prepare SIGINT handler
    signal(SIGINT, on_sigint);

    // Create UNIX domain socket
    const char *sockpath = argv[1];
    if (strlen(sockpath) >= sizeof(socket_path)) {
        fprintf(stderr, "Socket path too long\n");
        return 1;
    }
    strncpy(socket_path, sockpath, sizeof(socket_path)-1);
    unlink(socket_path); // in case it exists

    int serv = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serv < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(serv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(serv);
        return 1;
    }

    if (listen(serv, 5) < 0) {
        perror("listen");
        close(serv);
        unlink(socket_path);
        return 1;
    }

    set_nonblocking(serv);
    printf("SERVER: bind to socket file '%s'\n", socket_path);
    fflush(stdout);

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) client_init(&clients[i]);

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = NSLEEP_NS;

    while (running) {
        // Accept any new clients
        for (;;) {
            int cfd = accept(serv, NULL, NULL);
            if (cfd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                if (errno == EINTR) continue;
                perror("accept");
                break;
            }
            set_nonblocking(cfd);

            // Register client
            int placed = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = cfd;
                    clients[i].handshake_done = 0;
                    clients[i].inlen = 0;
                    clock_gettime(CLOCK_MONOTONIC, &clients[i].accepted_at);
                    // Send HELLO immediately (5 bytes, no terminator)
                    ssize_t w = send(cfd, HANDSHAKE_STR, HANDSHAKE_BYTES, 0);
                    (void)w;
                    placed = 1;
                    break;
                }
            }
            if (!placed) {
                // Too many clients; drop
                close(cfd);
            }
        }

        int any_activity = 0;

        // Service existing clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            Client *cl = &clients[i];
            if (cl->fd < 0) continue;

            // Check HELLO timeout
            if (!cl->handshake_done) {
                if (seconds_since(&cl->accepted_at) > 1.0) {
                    // Didn't receive HELLO back in time
                    client_close(cl);
                    continue;
                }
            }

            // Try to read
            char buf[512];
            ssize_t r = recv(cl->fd, buf, sizeof(buf), 0);
            if (r < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue; // no data
                } else if (errno == EINTR) {
                    continue;
                } else {
                    client_close(cl);
                    continue;
                }
            } else if (r == 0) {
                // Client closed
                client_close(cl);
                continue;
            } else {
                any_activity = 1;
                // Append to input buffer (truncate if overflow—simple lab handling)
                size_t copy = (size_t)r;
                if (cl->inlen + copy > INBUF_SIZE) {
                    copy = INBUF_SIZE - cl->inlen;
                }
                memcpy(cl->inbuf + cl->inlen, buf, copy);
                cl->inlen += copy;
            }

            // Process input
            if (!cl->handshake_done) {
                if (cl->inlen >= HANDSHAKE_BYTES &&
                    memcmp(cl->inbuf, HANDSHAKE_STR, HANDSHAKE_BYTES) == 0) {
                    // Consume exactly 5 bytes of HELLO
                    size_t remain = cl->inlen - HANDSHAKE_BYTES;
                    memmove(cl->inbuf, cl->inbuf + HANDSHAKE_BYTES, remain);
                    cl->inlen = remain;
                    cl->handshake_done = 1;
                }
                // else: wait for more bytes or timeout
                continue;
            }

            // After handshake, look for commands at start of buffer
            if (cl->inlen >= 3 && memcmp(cl->inbuf, "BYE", 3) == 0) {
                client_close(cl);
                continue;
            }

            if (cl->inlen >= 2 && cl->inbuf[0] == 'A' && cl->inbuf[1] == '\t') {
                // A\t<name>
                const char *name = (const char *)(cl->inbuf + 2);
                size_t name_len = cl->inlen - 2;
                handle_lookup(cl->fd, AF_INET, name, name_len);
                cl->inlen = 0; // clear buffer for next command
                continue;
            }

            if (cl->inlen >= 5 &&
                cl->inbuf[0] == 'A' && cl->inbuf[1] == 'A' &&
                cl->inbuf[2] == 'A' && cl->inbuf[3] == 'A' &&
                cl->inbuf[4] == '\t') {
                // AAAA\t<name>
                const char *name = (const char *)(cl->inbuf + 5);
                size_t name_len = cl->inlen - 5;
                handle_lookup(cl->fd, AF_INET6, name, name_len);
                cl->inlen = 0;
                continue;
            }

            // If no recognizable command yet, keep accumulating bytes.
        }

        if (!any_activity) {
            // Sleep briefly to avoid busy-waiting
            struct timespec rem;
            nanosleep(&req, &rem);
        }
    }

    // Graceful shutdown
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd >= 0) client_close(&clients[i]);
    }
    close(serv);
    unlink(socket_path);
    return 0;
}
