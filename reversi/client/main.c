#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "network.h"
#include "ui.h"
#include "client.h"
#include "../common/protocol.h"

static int g_socket_fd = -1;
static volatile sig_atomic_t g_should_quit = 0;

void handle_sigint(int sig) {
    (void)sig;
    g_should_quit = 1;
    if (g_socket_fd >= 0) {
        send_quit(g_socket_fd);
        close(g_socket_fd);
        g_socket_fd = -1;
    }
    printf("\nDisconnected from server.\n");
    exit(0);
}

int handle_server_message(const char *message) {
    MessageType msg_type = parse_message_type(message);
    char buffer[MAX_MESSAGE_LENGTH];
    char color[64];
    char result[64];
    char winner[64];
    int row;
    int col;
    int black_count;
    int white_count;
    
    switch (msg_type) {
        case MESSAGE_TYPE_WAIT:
            display_waiting();
            break;
            
        case MESSAGE_TYPE_WELCOME:
            if (parse_welcome_message(message, color) == 0) {
                display_welcome(color);
            }
            break;
            
        case MESSAGE_TYPE_START:
            display_status("Game starting!");
            break;
            
        case MESSAGE_TYPE_BOARD:
            if (parse_board_message(message, buffer) == 0) {
                display_board(buffer);
            }
            break;
            
        case MESSAGE_TYPE_YOUR_TURN:
            display_status("Your turn!");
            return 1;
            
        case MESSAGE_TYPE_OPPONENT_TURN:
            display_status("Opponent's turn...");
            break;
            
        case MESSAGE_TYPE_VALID:
            display_status("Move accepted!");
            break;
            
        case MESSAGE_TYPE_INVALID:
            if (parse_invalid_message(message, buffer) == 0) {
                display_error(buffer);
            } else {
                display_error("Invalid move");
            }
            return 1;
            
        case MESSAGE_TYPE_OPPONENT_MOVE:
            if (parse_opponent_move_message(message, &row, &col) == 0) {
                display_opponent_move(row, col);
            }
            break;
            
        case MESSAGE_TYPE_OPPONENT_PASS:
            display_status("Opponent passed");
            break;
            
        case MESSAGE_TYPE_GAME_OVER:
            if (parse_game_over_message(message, result, winner, &black_count, &white_count) == 0) {
                display_game_over(result, winner, black_count, white_count);
            }
            return -1;
            
        case MESSAGE_TYPE_OPPONENT_LEFT:
            display_status("Opponent has left the game");
            return -1;
            
        case MESSAGE_TYPE_ERROR:
            if (parse_error_message(message, buffer) == 0) {
                display_error(buffer);
            } else {
                display_error("Server error");
            }
            break;
            
        case MESSAGE_TYPE_UNKNOWN:
            printf("Unknown message: %s\n", message);
            break;
            
        default:
            break;
    }
    
    return 0;
}

int wait_for_server_message(int socket_fd, char *buffer, size_t buffer_size, int timeout_seconds) {
    fd_set read_fds;
    struct timeval timeout;
    
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);
    
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    
    int select_result = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);
    
    if (select_result < 0) {
        perror("select failed");
        return -1;
    }
    
    if (select_result == 0) {
        return 0;
    }
    
    ssize_t bytes_received = receive_server_message(socket_fd, buffer, buffer_size);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Server disconnected\n");
        } else {
            perror("recv failed");
        }
        return -1;
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    
    const char *host = argv[1];
    const char *port = argv[2];
    
    signal(SIGINT, handle_sigint);
    
    printf("Connecting to server at %s:%s...\n", host, port);
    
    g_socket_fd = connect_to_server(host, port);
    if (g_socket_fd < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return 1;
    }
    
    printf("Connected successfully!\n");
    
    char message_buffer[MAX_MESSAGE_LENGTH];
    int game_active = 1;
    int waiting_for_turn = 0;
    
    while (game_active && !g_should_quit) {
        if (waiting_for_turn) {
            int row;
            int col;
            int move_result = get_player_move(&row, &col);
            
            if (move_result == -1) {
                send_quit(g_socket_fd);
                break;
            } else if (move_result == 0) {
                send_pass(g_socket_fd);
            } else {
                send_move(g_socket_fd, row, col);
            }
            
            waiting_for_turn = 0;
        }
        
        int wait_result = wait_for_server_message(g_socket_fd, message_buffer, sizeof(message_buffer), 1);
        
        if (wait_result < 0) {
            game_active = 0;
            break;
        }
        
        if (wait_result == 0) {
            continue;
        }
        
        int handle_result = handle_server_message(message_buffer);
        
        if (handle_result == 1) {
            waiting_for_turn = 1;
        } else if (handle_result == -1) {
            game_active = 0;
        }
    }
    
    close(g_socket_fd);
    g_socket_fd = -1;
    
    printf("Disconnected from server.\n");
    return 0;
}
