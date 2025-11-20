#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

// This file WILL be graded.

// The Run button will compile both server.c and test.c.
// It will then run this server, and run the tester
// to connect to it.
// The server will be placed into the background and sent
// the SIGINT signal after 2 seconds.
// Your program will be killed after 5 seconds.

char *path = NULL;
int run = 1;
int server = -1;

typedef struct {
    int fd;
    char buffer[1024];
    int sentHello;
    int recieveHello;
    int active;
    size_t bufferL;
    time_t time;
} Client;

Client clients[100];

void handler(int signal) {
    (void)signal;
    run = 0;
}

uint16_t sBytes(uint16_t value) {
    return ntohs(value);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <socket file>\n", argv[0]);
        return 1;
    }

    path = argv[1];
    int i;
    int flags;
    int active;
    int connect;

    // initalize client slots
    for (i = 0; i < 100; i+=1) {
        clients[i].fd = -1;
        clients[i].time = 0;
        clients[i].sentHello = 0;
        clients[i].recieveHello = 0;
        clients[i].bufferL = 0;
        clients[i].active = 0;
        memset(clients[i].buffer, 0, sizeof(clients[i].buffer));
    }

    signal(SIGINT, handler);

    //Create non IP socket
    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server < 0) {
        return 1;
    }

    flags = fcntl(server, F_GETFL, 0);
    if (flags < 0) {
        return 1;

    }

    if (fcntl(server, F_SETFL, flags | O_NONBLOCK) < 0) {
        return 1;
    } 

    unlink(path);
    struct sockaddr_un address;

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path, sizeof(address.sun_path) - 1);
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) <  0){
        return 1;
    }

    // incoming connections
    listen(server, 5);

    printf("SERVER: bind to socket file '%s'\n", path);

    struct timespec sleep;
    sleep.tv_sec = 0;
    sleep.tv_nsec = 100000;

    while(run) {
        active = 0;
        int a;
        time_t currentTime = time(NULL);
        // accept new clinet
        int cfiled = accept(server, NULL, NULL);
        if (cfiled >= 0) {
            active = 1;
            connect = 0;

            flags = fcntl(cfiled, F_GETFL, 0);
            if (flags != - 1) {
                fcntl(cfiled, F_SETFL, flags | O_NONBLOCK);
            }


    // find open client slot
            for (a = 0; a < 100; a += 1) {
                if (!clients[a].active){
                    clients[a].fd = cfiled;
                    clients[a].active = 1;
                    clients[a].time = time(NULL);
                    clients[a].bufferL = 0;
                    clients[a].sentHello = 0;
                    clients[a].recieveHello = 0;
                    memset(clients[a].buffer, 0, sizeof(clients[a].buffer));
                    send(clients[a].fd, "HELLO", 5, 0);
                    clients[a].sentHello = 1;
                    connect = 1;
                    break;
                }
            }

            if (!connect) {
                close(cfiled);
            }
        }

        for (a = 0; a < 100; a += 1) {
            int requestF;
            char temp_buff[4096];

            if (!clients[a].active) {
                continue;
            }




            if ((!clients[a].recieveHello) && (clients[a].sentHello)) {
                if(currentTime - clients[a].time >= 1) {
                    close(clients[a].fd);
                    clients[a].active = 0;
                    clients[a].fd = -1;
                    continue;
                }
            }

            ssize_t bytes = recv(clients[a].fd, temp_buff, sizeof(temp_buff), 0);
            if (bytes <= 0) {
                if (bytes == 0) {
                    close(clients[a].fd);
                    clients[a].active = 0;
                    clients[a].fd = -1;
                }
                else {
                    temp_buff[bytes] = '\0';
                }
                continue;
            }

            active = 1;

            if (!clients[a].recieveHello) {
                if ((bytes >= 5) && (memcmp(temp_buff, "HELLO", 5) == 0)) {
                    clients[a].recieveHello = 1;
                }
                else {
                    close(clients[a].fd);
                    clients[a].active = 0;
                    clients[a].fd = -1;

                }
                continue;
            }

            if (memcmp(temp_buff, "BYE", 3 ) == 0 && bytes >= 3) {
                close(clients[a].fd);
                clients[a].active = 0;
                clients[a].fd = -1;
                continue;
            }

            if ((bytes >= 1 ) && (temp_buff[0] == 'A')) {
                char *gTab;
                requestF = 0;
                int ip = 0;

                if (temp_buff[1] == '\t') {
                    requestF = 1;
                    ip = 1;
                }
                else if ((bytes >= 5) && (memcmp(temp_buff, "AAAA\t", 5) == 0)) {
                    requestF = 1;
                    ip = 0;
                }

                if (requestF) {
                    gTab = strchr(temp_buff, '\t');
                    if (gTab) {
                        char hostname[256];
                        size_t length = bytes - (gTab - temp_buff) - 1;

                        if (sizeof(hostname) > length) {
                            memcpy(hostname, gTab + 1, length);
                            hostname[length] = '\0';


                            struct addrinfo *results;
                            struct addrinfo hints;
                            memset(&hints, 0, sizeof(hints));

                            if (ip == 1) {
                                hints.ai_family = AF_INET;

                            }
                            else {
                                hints.ai_family = AF_INET6;
                            }

                            hints.ai_socktype = SOCK_STREAM;

                            int stat = getaddrinfo(hostname, NULL, &hints, &results);
                            if (stat == 0 && results != NULL) {
                                char response[1024];



                                if (ip > 0 ) {
                                    // format correctly
                                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)results->ai_addr;
                                    unsigned char *bytes_addr = (unsigned char *)&(ipv4->sin_addr);
    snprintf(response, sizeof(response), "OK\t%03d.%03d.%03d.%03d", bytes_addr[0], bytes_addr[1], bytes_addr[2], bytes_addr[3]);
    send(clients[a].fd, response, strlen(response), 0);
                                }
    else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)results->ai_addr;
    uint16_t *word = (uint16_t *)&(ipv6->sin6_addr);

    snprintf(response, sizeof(response),
    // I utilized ChatGPT in order to figure out how to format correctly
             "OK\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
             (unsigned)sBytes(word[0]), (unsigned)sBytes(word[1]),
             (unsigned)sBytes(word[2]), (unsigned)sBytes(word[3]),
             (unsigned)sBytes(word[4]), (unsigned)sBytes(word[5]),
             (unsigned)sBytes(word[6]), (unsigned)sBytes(word[7]));

    send(clients[a].fd, response, strlen(response), 0);
}

    freeaddrinfo(results);
                            }
                            else {
                                send(clients[a].fd, "NOTFOUND", 8, 0);
                            }
                        }
                        else {
                            send(clients[a].fd, "NOTFOUND", 8, 0);
                        }
                    }
                    else {
                        send(clients[a].fd, "NOTFOUND", 8, 0);
                    }

                    memset(clients[a].buffer, 0, sizeof(clients[a].buffer));
                    clients[a].bufferL = 0;
                }
            }
        }

        if (!active) {
            nanosleep(&sleep, NULL);
        }
    }


for (i = 0; i <100; i += 1) {
    if ((clients[i].active > 0) && (clients[i].fd >= 0 )) {
        close(clients[i].fd);
    }
}

unlink(path);
close(server);
return 0;


}
