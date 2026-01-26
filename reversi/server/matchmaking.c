#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include "matchmaking.h"
#include "network.h"
#include "game.h"
#include "../common/protocol.h"

#define MAX_WAITING_PLAYERS 100

static int waiting_players_queue[MAX_WAITING_PLAYERS];
static int waiting_players_count = 0;

void initialize_matchmaking(void) {
    waiting_players_count = 0;
}

void add_waiting_player(int client_socket) {
    if (waiting_players_count < MAX_WAITING_PLAYERS) {
        waiting_players_queue[waiting_players_count] = client_socket;
        waiting_players_count++;
    }
}

int has_waiting_players(void) {
    return waiting_players_count > 0;
}

static int get_waiting_player(void) {
    if (waiting_players_count <= 0) {
        return -1;
    }
    
    int player_socket = waiting_players_queue[0];
    
    for (int i = 0; i < waiting_players_count - 1; i++) {
        waiting_players_queue[i] = waiting_players_queue[i + 1];
    }
    waiting_players_count--;
    
    return player_socket;
}

static void handle_paired_game(int black_player_socket, int white_player_socket) {
    GameState game;
    initialize_game(&game);
    
    send_board_message(black_player_socket, &game);
    send_board_message(white_player_socket, &game);
    
    char buffer[MAX_MESSAGE_LENGTH];
    int first_turn = 1;
    
    while (!is_game_over(&game)) {
        int current_socket = (game.current_player == PLAYER_BLACK) ? black_player_socket : white_player_socket;
        int opponent_socket = (game.current_player == PLAYER_BLACK) ? white_player_socket : black_player_socket;
        
        if (!has_legal_moves(&game, game.current_player)) {
            if (send_opponent_pass_message(opponent_socket) < 0) {
                printf("Player disconnected\n");
                send_opponent_left_message(current_socket);
                return;
            }
            game.current_player = (game.current_player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
            first_turn = 1;
            continue;
        }
        
        if (first_turn) {
            send_your_turn_message(current_socket);
            if (send_opponent_turn_message(opponent_socket) < 0) {
                printf("Player disconnected\n");
                send_opponent_left_message(current_socket);
                return;
            }
            first_turn = 0;
        }
        
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(black_player_socket, &read_fds);
        FD_SET(white_player_socket, &read_fds);
        
        int max_fd = (black_player_socket > white_player_socket) ?
                      black_player_socket : white_player_socket;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            printf("select error\n");
            send_opponent_left_message(current_socket);
            send_opponent_left_message(opponent_socket);
            return;
        }
        
        if (FD_ISSET(opponent_socket, &read_fds)) {
            char discard_buffer[MAX_MESSAGE_LENGTH];
            ssize_t bytes = recv(opponent_socket, discard_buffer, sizeof(discard_buffer), 0);
            
            if (bytes <= 0) {
                send_opponent_left_message(current_socket);
                printf("Player disconnected\n");
                return;
            }
            
            continue;
        }
        
        if (!FD_ISSET(current_socket, &read_fds)) {
            continue;
        }
        
        ssize_t bytes_received = receive_message(current_socket, buffer, sizeof(buffer));
        
        if (bytes_received <= 0) {
            if (bytes_received == 0 || (bytes_received < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                printf("Player disconnected\n");
                send_opponent_left_message(opponent_socket);
                return;
            }
        }
        
        if (is_quit_message(buffer)) {
            printf("Player disconnected\n");
            send_opponent_left_message(opponent_socket);
            return;
        }
        
        if (is_pass_message(buffer)) {
            if (!has_legal_moves(&game, game.current_player)) {
                send_valid_message(current_socket);
                if (send_opponent_pass_message(opponent_socket) < 0) {
                    printf("Player disconnected\n");
                    send_opponent_left_message(current_socket);
                    return;
                }
                game.current_player = (game.current_player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
                first_turn = 1;
            } else {
                send_invalid_message(current_socket, "has_legal_moves");
            }
            continue;
        }
        
        int row, col;
        if (parse_move_message(buffer, &row, &col)) {
            if (row < 0 || row >= BOARD_HEIGHT || col < 0 || col >= BOARD_WIDTH) {
                send_invalid_message(current_socket, "out_of_bounds");
                continue;
            }
            
            if (game.board[row][col] != CELL_EMPTY) {
                send_invalid_message(current_socket, "occupied");
                continue;
            }
            
            if (!is_valid_move(&game, row, col)) {
                send_invalid_message(current_socket, "no_flip");
                continue;
            }
            
            if (execute_move(&game, row, col)) {
                send_valid_message(current_socket);
                if (send_opponent_move_message(opponent_socket, row, col) < 0) {
                    printf("Player disconnected\n");
                    send_opponent_left_message(current_socket);
                    return;
                }
                if (send_board_message(black_player_socket, &game) < 0 ||
                    send_board_message(white_player_socket, &game) < 0) {
                    printf("Player disconnected\n");
                    int remaining_socket = (send_board_message(black_player_socket, &game) < 0) ?
                                           white_player_socket : black_player_socket;
                    send_opponent_left_message(remaining_socket);
                    return;
                }
                first_turn = 1;
            } else {
                send_invalid_message(current_socket, "no_flip");
            }
        } else {
            send_invalid_message(current_socket, "unknown_command");
        }
    }
    
    int black_count, white_count;
    count_pieces(&game, &black_count, &white_count);
    GameStatus status = determine_winner(&game);
    
    const char *result;
    const char *winner_color;
    
    if (status == GAME_STATUS_BLACK_WINS) {
        result = "WIN";
        winner_color = COLOR_BLACK;
    } else if (status == GAME_STATUS_WHITE_WINS) {
        result = "WIN";
        winner_color = COLOR_WHITE;
    } else {
        result = "DRAW";
        winner_color = "NONE";
    }
    
    send_game_over_message(black_player_socket, result, winner_color, black_count, white_count);
    send_game_over_message(white_player_socket, result, winner_color, black_count, white_count);
}

static void pair_and_start_game(int black_player_socket, int white_player_socket) {
    send_welcome_message(black_player_socket, COLOR_BLACK);
    send_welcome_message(white_player_socket, COLOR_WHITE);
    
    send_start_message(black_player_socket);
    send_start_message(white_player_socket);
    
    pid_t child_process_id = fork();
    if (child_process_id < 0) {
        perror("fork failed for game pair");
        close(black_player_socket);
        close(white_player_socket);
        return;
    }
    
    if (child_process_id == 0) {
        handle_paired_game(black_player_socket, white_player_socket);
        close(black_player_socket);
        close(white_player_socket);
        exit(EXIT_SUCCESS);
    } else {
        close(black_player_socket);
        close(white_player_socket);
    }
}

static int try_pair_players(void) {
    if (waiting_players_count < 2) {
        return 0;
    }
    
    int black_player_socket = get_waiting_player();
    int white_player_socket = get_waiting_player();
    
    if (black_player_socket >= 0 && white_player_socket >= 0) {
        pair_and_start_game(black_player_socket, white_player_socket);
        return 1;
    }
    
    return 0;
}

void handle_new_connection(int client_socket) {
    add_waiting_player(client_socket);
    
    if (waiting_players_count >= 2) {
        try_pair_players();
    } else {
        send_wait_message(client_socket);
    }
}
