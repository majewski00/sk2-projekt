#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ui.h"
#include "../common/board.h"

void display_board(const char *board_string) {
    printf("\n    0 1 2 3 4 5 6 7\n");
    printf("  +----------------\n");
    
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        printf("%d | ", row);
        for (int col = 0; col < BOARD_WIDTH; col++) {
            int index = row * BOARD_WIDTH + col;
            printf("%c ", board_string[index]);
        }
        printf("\n");
    }
    printf("\n");
}

void display_status(const char *message) {
    printf(">>> %s\n", message);
}

void display_welcome(const char *color) {
    printf("\n");
    printf("====================================\n");
    printf("    WELCOME TO REVERSI\n");
    printf("====================================\n");
    printf("You are playing as: %s\n", color);
    printf("====================================\n");
    printf("\n");
}

void display_waiting(void) {
    printf("Waiting for opponent to connect...\n");
}

int get_player_move(int *row, int *col) {
    char input[256];
    
    printf("Your turn! Enter move (e.g., '3 4' or 'd3'), 'pass', or 'quit': ");
    fflush(stdout);
    
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return -1;
    }
    
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }
    
    if (strcasecmp(input, "quit") == 0 || strcasecmp(input, "q") == 0) {
        return -1;
    }
    
    if (strcasecmp(input, "pass") == 0 || strcasecmp(input, "p") == 0) {
        return 0;
    }
    
    if (parse_move_input(input, row, col) == 0) {
        return 1;
    }
    
    printf("Invalid input format. Use 'row col' (e.g., '3 4') or algebraic notation (e.g., 'd3')\n");
    return get_player_move(row, col);
}

int parse_move_input(const char *input, int *row, int *col) {
    int parsed_row;
    int parsed_col;
    
    if (sscanf(input, "%d %d", &parsed_row, &parsed_col) == 2) {
        if (parsed_row >= 0 && parsed_row < BOARD_HEIGHT && 
            parsed_col >= 0 && parsed_col < BOARD_WIDTH) {
            *row = parsed_row;
            *col = parsed_col;
            return 0;
        }
        return -1;
    }
    
    if (strlen(input) >= 2) {
        char col_char = tolower(input[0]);
        char row_char = input[1];
        
        if (col_char >= 'a' && col_char <= 'h' && row_char >= '0' && row_char <= '7') {
            *col = col_char - 'a';
            *row = row_char - '0';
            return 0;
        }
    }
    
    return -1;
}

void display_error(const char *message) {
    printf("ERROR: %s\n", message);
}

void display_opponent_move(int row, int col) {
    printf("Opponent played at position: %d %d\n", row, col);
}

void display_game_over(const char *result, const char *winner, int black_count, int white_count) {
    printf("\n");
    printf("====================================\n");
    printf("           GAME OVER\n");
    printf("====================================\n");
    printf("Result: %s\n", result);
    printf("Winner: %s\n", winner);
    printf("Final Score:\n");
    printf("  Black: %d\n", black_count);
    printf("  White: %d\n", white_count);
    printf("====================================\n");
    printf("\n");
}
