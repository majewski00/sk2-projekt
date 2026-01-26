#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "network.h"
#include "../common/protocol.h"

int connect_to_server(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;
    int socket_fd;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(host, port, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        return -1;
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1) {
            continue;
        }
        
        if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        
        close(socket_fd);
    }
    
    freeaddrinfo(result);
    
    if (rp == NULL) {
        fprintf(stderr, "Could not connect to server\n");
        return -1;
    }
    
    return socket_fd;
}

ssize_t send_client_message(int socket_fd, const char *message) {
    size_t message_length = strlen(message);
    return send(socket_fd, message, message_length, 0);
}

ssize_t receive_server_message(int socket_fd, char *buffer, size_t buffer_size) {
    size_t total_received = 0;
    
    while (total_received < buffer_size - 1) {
        ssize_t bytes_received = recv(socket_fd, buffer + total_received, 1, 0);
        
        if (bytes_received <= 0) {
            if (total_received == 0) {
                return bytes_received;
            }
            break;
        }
        
        if (buffer[total_received] == '\n') {
            buffer[total_received] = '\0';
            return total_received;
        }
        
        total_received++;
    }
    
    buffer[total_received] = '\0';
    return total_received;
}

int send_move(int socket_fd, int row, int col) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s%d%s%d%s",
             MESSAGE_MOVE, PROTOCOL_DELIMITER, row, PROTOCOL_DELIMITER, col, PROTOCOL_TERMINATOR);
    return send_client_message(socket_fd, message) > 0 ? 0 : -1;
}

int send_pass(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_PASS, PROTOCOL_TERMINATOR);
    return send_client_message(socket_fd, message) > 0 ? 0 : -1;
}

int send_quit(int socket_fd) {
    char message[MAX_MESSAGE_LENGTH];
    snprintf(message, sizeof(message), "%s%s", MESSAGE_QUIT, PROTOCOL_TERMINATOR);
    return send_client_message(socket_fd, message) > 0 ? 0 : -1;
}

MessageType parse_message_type(const char *message) {
    if (strncmp(message, MESSAGE_WAIT, strlen(MESSAGE_WAIT)) == 0) {
        return MESSAGE_TYPE_WAIT;
    }
    if (strncmp(message, MESSAGE_WELCOME, strlen(MESSAGE_WELCOME)) == 0) {
        return MESSAGE_TYPE_WELCOME;
    }
    if (strncmp(message, MESSAGE_START, strlen(MESSAGE_START)) == 0) {
        return MESSAGE_TYPE_START;
    }
    if (strncmp(message, MESSAGE_BOARD, strlen(MESSAGE_BOARD)) == 0) {
        return MESSAGE_TYPE_BOARD;
    }
    if (strncmp(message, MESSAGE_YOUR_TURN, strlen(MESSAGE_YOUR_TURN)) == 0) {
        return MESSAGE_TYPE_YOUR_TURN;
    }
    if (strncmp(message, MESSAGE_OPPONENT_TURN, strlen(MESSAGE_OPPONENT_TURN)) == 0) {
        return MESSAGE_TYPE_OPPONENT_TURN;
    }
    if (strncmp(message, MESSAGE_VALID, strlen(MESSAGE_VALID)) == 0) {
        return MESSAGE_TYPE_VALID;
    }
    if (strncmp(message, MESSAGE_INVALID, strlen(MESSAGE_INVALID)) == 0) {
        return MESSAGE_TYPE_INVALID;
    }
    if (strncmp(message, MESSAGE_OPPONENT_MOVE, strlen(MESSAGE_OPPONENT_MOVE)) == 0) {
        return MESSAGE_TYPE_OPPONENT_MOVE;
    }
    if (strncmp(message, MESSAGE_OPPONENT_PASS, strlen(MESSAGE_OPPONENT_PASS)) == 0) {
        return MESSAGE_TYPE_OPPONENT_PASS;
    }
    if (strncmp(message, MESSAGE_GAME_OVER, strlen(MESSAGE_GAME_OVER)) == 0) {
        return MESSAGE_TYPE_GAME_OVER;
    }
    if (strncmp(message, MESSAGE_OPPONENT_LEFT, strlen(MESSAGE_OPPONENT_LEFT)) == 0) {
        return MESSAGE_TYPE_OPPONENT_LEFT;
    }
    if (strncmp(message, MESSAGE_ERROR, strlen(MESSAGE_ERROR)) == 0) {
        return MESSAGE_TYPE_ERROR;
    }
    return MESSAGE_TYPE_UNKNOWN;
}

int parse_welcome_message(const char *message, char *color) {
    const char *delimiter_pos = strchr(message, '|');
    if (delimiter_pos == NULL) {
        return -1;
    }
    
    strcpy(color, delimiter_pos + 1);
    return 0;
}

int parse_board_message(const char *message, char *board) {
    const char *delimiter_pos = strchr(message, '|');
    if (delimiter_pos == NULL) {
        return -1;
    }
    
    strcpy(board, delimiter_pos + 1);
    return 0;
}

int parse_invalid_message(const char *message, char *reason) {
    const char *delimiter_pos = strchr(message, '|');
    if (delimiter_pos == NULL) {
        strcpy(reason, "Unknown reason");
        return -1;
    }
    
    strcpy(reason, delimiter_pos + 1);
    return 0;
}

int parse_opponent_move_message(const char *message, int *row, int *col) {
    if (sscanf(message, "OPPONENT_MOVE|%d|%d", row, col) == 2) {
        return 0;
    }
    return -1;
}

int parse_game_over_message(const char *message, char *result, char *winner, int *black_count, int *white_count) {
    if (sscanf(message, "GAME_OVER|%[^|]|%[^|]|%d|%d", result, winner, black_count, white_count) == 4) {
        return 0;
    }
    return -1;
}

int parse_error_message(const char *message, char *error_text) {
    const char *delimiter_pos = strchr(message, '|');
    if (delimiter_pos == NULL) {
        strcpy(error_text, "Unknown error");
        return -1;
    }
    
    strcpy(error_text, delimiter_pos + 1);
    return 0;
}
