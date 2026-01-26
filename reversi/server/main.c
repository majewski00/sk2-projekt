#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "server.h"
#include "network.h"
#include "matchmaking.h"

void handle_sigchld(int signal) {
    (void)signal;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int create_server_socket(uint16_t port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuse_option = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_option, sizeof(reuse_option)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG_SIZE) < 0) {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void accept_clients(int server_socket) {
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = handle_sigchld;
    sigemptyset(&signal_action.sa_mask);
    signal_action.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &signal_action, NULL) < 0) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port...\n");
    initialize_matchmaking();

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept failed");
            continue;
        }

        printf("Client connected from %s:%d\n",
               inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port));

        handle_new_connection(client_socket);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr;
    long port_number = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || port_number <= 0 || port_number > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return EXIT_FAILURE;
    }

    int server_socket = create_server_socket((uint16_t)port_number);
    accept_clients(server_socket);
    close(server_socket);

    return EXIT_SUCCESS;
}
