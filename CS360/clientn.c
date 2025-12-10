#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <hostname> <port> <path>\n", argv[0]);
        return 1;
    }

    const char *hostname = argv[1];
    const char *port = argv[2];
    const char *path = argv[3];

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Get host information
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port, &hints, &result) != 0) {
        perror("getaddrinfo");
        close(sockfd);
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        perror("connect");
        freeaddrinfo(result);
        close(sockfd);
        return 1;
    }

    freeaddrinfo(result);

    // Build HTTP request
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, hostname);

    // Send HTTP request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send");
        close(sockfd);
        return 1;
    }

    // Receive response
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int header_end_found = 0;
    char *data_start = NULL;
    
    // We'll accumulate the response to find where headers end
    char *response = NULL;
    size_t response_size = 0;
    
    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) {
        // Append to response
        response = realloc(response, response_size + bytes_received);
        memcpy(response + response_size, buffer, bytes_received);
        response_size += bytes_received;
        
        // Look for end of headers if not found yet
        if (!header_end_found) {
            // Search for \r\n\r\n
            for (size_t i = 0; i < response_size - 3; i++) {
                if (response[i] == '\r' && response[i+1] == '\n' && 
                    response[i+2] == '\r' && response[i+3] == '\n') {
                    header_end_found = 1;
                    data_start = response + i + 4;
                    // Print everything after headers
                    fwrite(data_start, 1, response_size - (data_start - response), stdout);
                    break;
                }
            }
        } else {
            // Headers already found, just print the data
            fwrite(buffer, 1, bytes_received, stdout);
        }
    }

    if (response) {
        free(response);
    }
    close(sockfd);
    return 0;
}