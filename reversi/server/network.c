// #define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include "network.h"
#include "../common/protocol.h"
#include "../common/board.h"

#define BUFFER_SIZE 1024

ssize_t receive_message(int socket_fd, char *buffer, size_t buffer_size) {
    ssize_t bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
    }
    return bytes_received;
}

ssize_t send_message(int socket_fd, const char *message, size_t message_length) {
    return send(socket_fd, message, message_length, 0);
}

void handle_client_connection(int client_socket) {
    char receive_buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = receive_message(client_socket, receive_buffer, BUFFER_SIZE)) > 0) {
        printf("Received %zd bytes\n", bytes_received);
        
        ssize_t bytes_sent = send_message(client_socket, receive_buffer, bytes_received);
        if (bytes_sent < 0) {
            perror("send failed");
            break;
        }
    }

    if (bytes_received < 0) {
        perror("recv failed");
    } else if (bytes_received == 0) {
        printf("Client disconnected\n");
    }
}

ssize_t send_wait_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_WAIT, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_welcome_message(int socket_fd, const char *color) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s%s%s",
             MESSAGE_WELCOME, PROTOCOL_DELIMITER, color, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_start_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_START, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_board_message(int socket_fd, const GameState *game) {
    char message[MAX_MESSAGE_LENGTH];
    char board_string[BOARD_SIZE + 1];
    
    int index = 0;
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            board_string[index++] = game->board[row][col];
        }
    }
    board_string[BOARD_SIZE] = '\0';
    
    snprintf(message, sizeof(message), "%s%s%s%s",
             MESSAGE_BOARD, PROTOCOL_DELIMITER, board_string, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_your_turn_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_YOUR_TURN, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_opponent_turn_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_OPPONENT_TURN, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_valid_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_VALID, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_invalid_message(int socket_fd, const char *reason) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s%s%s",
             MESSAGE_INVALID, PROTOCOL_DELIMITER, reason, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_opponent_move_message(int socket_fd, int row, int col) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s%d%s%d%s",
             MESSAGE_OPPONENT_MOVE, PROTOCOL_DELIMITER, row, PROTOCOL_DELIMITER, col, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_opponent_pass_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_OPPONENT_PASS, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_game_over_message(int socket_fd, const char *result, const char *winner_color, int black_count, int white_count) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s%s%s%s%s%d%s%d%s",
             MESSAGE_GAME_OVER, PROTOCOL_DELIMITER, result, PROTOCOL_DELIMITER, winner_color,
             PROTOCOL_DELIMITER, black_count, PROTOCOL_DELIMITER, white_count, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

ssize_t send_opponent_left_message(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_OPPONENT_LEFT, PROTOCOL_TERMINATOR);
    return send_message(socket_fd, message, strlen(message));
}

int parse_move_message(const char *message, int *row, int *col) {
    char command[32];
    if (sscanf(message, "%[^|]|%d|%d", command, row, col) == 3) {
        if (strcasecmp(command, MESSAGE_MOVE) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_pass_message(const char *message) {
    char trimmed[MAX_MESSAGE_LENGTH];
    int i = 0;
    while (message[i] && !isspace((unsigned char)message[i]) && i < MAX_MESSAGE_LENGTH - 1) {
        trimmed[i] = message[i];
        i++;
    }
    trimmed[i] = '\0';
    return strcasecmp(trimmed, MESSAGE_PASS) == 0;
}

int is_quit_message(const char *message) {
    char trimmed[MAX_MESSAGE_LENGTH];
    int i = 0;
    while (message[i] && !isspace((unsigned char)message[i]) && i < MAX_MESSAGE_LENGTH - 1) {
        trimmed[i] = message[i];
        i++;
    }
    trimmed[i] = '\0';
    return strcasecmp(trimmed, MESSAGE_QUIT) == 0;
}
