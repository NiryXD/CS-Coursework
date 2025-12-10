#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

// Global variables for cleanup
int server_fd = -1;
char *socket_path = NULL;
int client_fds[MAX_CLIENTS];
int num_clients = 0;

// Client structure to track state
typedef struct {
    int fd;
    int hello_sent;
    int hello_received;
    time_t hello_time;
} Client;

Client clients[MAX_CLIENTS];

// Set socket to non-blocking mode
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Signal handler for SIGINT
void sigint_handler(int sig) {
    (void)sig;  // Suppress unused parameter warning
    
    // Close all client connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd >= 0) {
            close(clients[i].fd);
        }
    }
    
    // Close server socket
    if (server_fd >= 0) {
        close(server_fd);
    }
    
    // Unlink socket file
    if (socket_path) {
        unlink(socket_path);
    }
    
    exit(0);
}

// Initialize client array
void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].hello_sent = 0;
        clients[i].hello_received = 0;
        clients[i].hello_time = 0;
    }
}

// Add a new client
int add_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd < 0) {
            clients[i].fd = fd;
            clients[i].hello_sent = 0;
            clients[i].hello_received = 0;
            clients[i].hello_time = time(NULL);
            return i;
        }
    }
    return -1;
}

// Remove a client
void remove_client(int index) {
    if (index >= 0 && index < MAX_CLIENTS && clients[index].fd >= 0) {
        close(clients[index].fd);
        clients[index].fd = -1;
        clients[index].hello_sent = 0;
        clients[index].hello_received = 0;
        clients[index].hello_time = 0;
    }
}

// Handle DNS lookup
void handle_lookup(int client_fd, const char *cmd, const char *hostname) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    
    // Set address family based on command
    if (strcmp(cmd, "A") == 0) {
        hints.ai_family = AF_INET;
    } else if (strcmp(cmd, "AAAA") == 0) {
        hints.ai_family = AF_INET6;
    } else {
        return;
    }
    
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, NULL, &hints, &result) == 0 && result != NULL) {
        char response[BUFFER_SIZE];
        
        if (result->ai_family == AF_INET) {
            // IPv4 address
            struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
            unsigned char *bytes = (unsigned char *)&addr->sin_addr.s_addr;
            
            // Convert bytes to dotted decimal notation
            snprintf(response, sizeof(response), "OK\t%d.%d.%d.%d",
                    bytes[0], bytes[1], bytes[2], bytes[3]);
        } else if (result->ai_family == AF_INET6) {
            // IPv6 address
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)result->ai_addr;
            unsigned char *bytes = addr->sin6_addr.s6_addr;
            
            // Convert bytes to colon-separated hexadecimal notation
            snprintf(response, sizeof(response), 
                    "OK\t%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                    bytes[0], bytes[1], bytes[2], bytes[3],
                    bytes[4], bytes[5], bytes[6], bytes[7],
                    bytes[8], bytes[9], bytes[10], bytes[11],
                    bytes[12], bytes[13], bytes[14], bytes[15]);
        }
        
        send(client_fd, response, strlen(response), 0);
        
        freeaddrinfo(result);
    } else {
        send(client_fd, "NOTFOUND", 8, 0);
    }
}

// Process client data
void process_client(int index) {
    Client *client = &clients[index];
    char buffer[BUFFER_SIZE];
    
    // Send HELLO if not sent yet
    if (!client->hello_sent) {
        send(client->fd, "HELLO", 5, 0);
        client->hello_sent = 1;
        client->hello_time = time(NULL);
        return;
    }
    
    // Check for timeout on HELLO response
    if (client->hello_sent && !client->hello_received) {
        if (time(NULL) - client->hello_time > 1) {
            remove_client(index);
            return;
        }
    }
    
    // Try to read data
    ssize_t bytes = recv(client->fd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes <= 0) {
        if (bytes == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            remove_client(index);
        }
        return;
    }
    
    buffer[bytes] = '\0';
    
    // Check for HELLO response
    if (!client->hello_received && strncmp(buffer, "HELLO", 5) == 0) {
        client->hello_received = 1;
        return;
    }
    
    // Only process commands if HELLO handshake is complete
    if (!client->hello_received) {
        return;
    }
    
    // Process commands
    if (strncmp(buffer, "BYE", 3) == 0) {
        remove_client(index);
    } else if (strncmp(buffer, "A\t", 2) == 0) {
        handle_lookup(client->fd, "A", buffer + 2);
    } else if (strncmp(buffer, "AAAA\t", 5) == 0) {
        handle_lookup(client->fd, "AAAA", buffer + 5);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <socket file>\n", argv[0]);
        return 1;
    }
    
    socket_path = argv[1];
    
    // Set up signal handler
    signal(SIGINT, sigint_handler);
    
    // Initialize clients array
    init_clients();
    
    // Create UNIX socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }
    
    // Set non-blocking
    set_nonblocking(server_fd);
    
    // Set up address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    
    // Remove existing socket file if it exists
    unlink(socket_path);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }
    
    // Listen with backlog of 5
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        unlink(socket_path);
        return 1;
    }
    
    printf("SERVER: bind to socket file '%s'\n", socket_path);
    
    // Main server loop
    while (1) {
        int activity = 0;
        
        // Check for new connections
        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (new_fd >= 0) {
            set_nonblocking(new_fd);
            if (add_client(new_fd) < 0) {
                close(new_fd);  // Too many clients
            }
            activity = 1;
        }
        
        // Process existing clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0) {
                process_client(i);
                activity = 1;
            }
        }
        
        // Sleep if no activity
        if (!activity) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 100000;  // 100,000 nanoseconds
            nanosleep(&ts, NULL);
        }
    }
    
    return 0;
}