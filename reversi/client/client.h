#ifndef CLIENT_H
#define CLIENT_H

int handle_server_message(const char *message);
int wait_for_server_message(int socket_fd, char *buffer, size_t buffer_size, int timeout_seconds);
void handle_sigint(int sig);

#endif
