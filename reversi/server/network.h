#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include "game.h"

void handle_client_connection(int client_socket);
ssize_t receive_message(int socket_fd, char *buffer, size_t buffer_size);
ssize_t send_message(int socket_fd, const char *message, size_t message_length);
ssize_t send_wait_message(int socket_fd);
ssize_t send_welcome_message(int socket_fd, const char *color);
ssize_t send_start_message(int socket_fd);

ssize_t send_board_message(int socket_fd, const GameState *game);
ssize_t send_your_turn_message(int socket_fd);
ssize_t send_opponent_turn_message(int socket_fd);
ssize_t send_valid_message(int socket_fd);
ssize_t send_invalid_message(int socket_fd, const char *reason);
ssize_t send_opponent_move_message(int socket_fd, int row, int col);
ssize_t send_opponent_pass_message(int socket_fd);
ssize_t send_game_over_message(int socket_fd, const char *result, const char *winner_color, int black_count, int white_count);
ssize_t send_opponent_left_message(int socket_fd);

int parse_move_message(const char *message, int *row, int *col);
int is_pass_message(const char *message);
int is_quit_message(const char *message);

#endif
