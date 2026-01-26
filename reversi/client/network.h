#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include <stddef.h>
#include "../common/protocol.h"

int connect_to_server(const char *host, const char *port);
ssize_t send_client_message(int socket_fd, const char *message);
ssize_t receive_server_message(int socket_fd, char *buffer, size_t buffer_size);
int send_move(int socket_fd, int row, int col);
int send_pass(int socket_fd);
int send_quit(int socket_fd);
MessageType parse_message_type(const char *message);
int parse_welcome_message(const char *message, char *color);
int parse_board_message(const char *message, char *board);
int parse_invalid_message(const char *message, char *reason);
int parse_opponent_move_message(const char *message, int *row, int *col);
int parse_game_over_message(const char *message, char *result, char *winner, int *black_count, int *white_count);
int parse_error_message(const char *message, char *error_text);

#endif
