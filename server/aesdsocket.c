#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>

#define PORT 9000
#define SOCKET_PATH "/var/tmp/aesdsocketdata"
#define MAX_BUFFER_SIZE 1024

static int server_socket = -1;
static int client_socket = -1;
static FILE *file = NULL;

void cleanup_and_exit(int signum)
{
    (void)signum;  // Prevent unused parameter warning

    if (file) {
        fclose(file);
        file = NULL;
    }

    if (client_socket != -1) {
        close(client_socket);
    }

    if (server_socket != -1) {
        close(server_socket);
    }

    if (remove(SOCKET_PATH) == 0) {
        syslog(LOG_INFO, "Removed file: %s", SOCKET_PATH);
    }

    syslog(LOG_INFO, "Caught signal, exiting");

    exit(0);
}

int setup_server_socket(void)
{
    struct sockaddr_in server_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        syslog(LOG_ERR, "Error creating socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Error binding socket to port");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        syslog(LOG_ERR, "Error listening on socket");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void handle_client_connection(void)
{
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    size_t total_received = 0;
    
    while ((bytes_received = recv(client_socket, buffer + total_received, sizeof(buffer) - total_received, 0)) > 0) {
        total_received += bytes_received;
        
        if (buffer[total_received - 1] == '\n') {
            buffer[total_received] = '\0'; // Ensure null-termination

            // Append data to the file
            file = fopen(SOCKET_PATH, "a");
            if (file == NULL) {
                syslog(LOG_ERR, "Failed to open the file for appending");
                close(client_socket);
                return;
            }

            fputs(buffer, file);
            fclose(file);
            
            // Send the full content of the file back to the client
            file = fopen(SOCKET_PATH, "r");
            if (file == NULL) {
                syslog(LOG_ERR, "Failed to open file for reading");
                close(client_socket);
                return;
            }

            char file_content[MAX_BUFFER_SIZE];
            size_t n;
            while ((n = fread(file_content, 1, sizeof(file_content), file)) > 0) {
                send(client_socket, file_content, n, 0);
            }
            fclose(file);

            total_received = 0;  // Reset for next packet
        }
    }

    if (bytes_received < 0) {
        syslog(LOG_ERR, "Error receiving data from client");
    }

    close(client_socket);
    syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(((struct sockaddr_in*)&client_socket)->sin_addr));
}

int main(void)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct sigaction sa;

    openlog("aesdsocket", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting server...");

    // Set up signal handling for graceful shutdown
    sa.sa_handler = cleanup_and_exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    server_socket = setup_server_socket();
    if (server_socket == -1) {
        syslog(LOG_ERR, "Failed to set up server socket");
        return -1;
    }

    while (1) {
        syslog(LOG_INFO, "Waiting for connections...");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            syslog(LOG_ERR, "Error accepting connection");
            continue;
        }

        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(client_addr.sin_addr));

        handle_client_connection();
    }

    cleanup_and_exit(0);
    return 0;
}

