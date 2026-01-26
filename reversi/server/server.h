#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#define BACKLOG_SIZE 10

typedef struct {
    int socket_fd;
    uint16_t port;
} server_config;

int create_server_socket(uint16_t port);
void accept_clients(int server_socket);

#endif
