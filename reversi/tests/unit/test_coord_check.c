#include <stdio.h>
#include "server/game.h"

int main() {
    GameState game;
    initialize_game(&game);
    
    printf("=== Testing coordinate interpretation ===\n\n");
    
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        for (int c = 0; c < BOARD_WIDTH; c++) {
            game.board[r][c] = CELL_EMPTY;
        }
    }
    
    game.board[3][3] = CELL_WHITE;
    game.board[3][4] = CELL_WHITE;
    game.board[3][5] = CELL_WHITE;
    game.board[4][3] = CELL_BLACK;
    game.board[4][4] = CELL_BLACK;
    game.board[4][5] = CELL_BLACK;
    game.current_player = PLAYER_BLACK;
    
    printf("Board state:\n");
    printf("  0 1 2 3 4 5 6 7\n");
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        printf("%d ", r);
        for (int c = 0; c < BOARD_WIDTH; c++) {
            printf("%c ", game.board[r][c]);
        }
        printf("\n");
    }
    
    printf("\n=== Test 1: MOVE|2|3 parsed as row=2, col=3 ===\n");
    bool valid1 = is_valid_move(&game, 2, 3);
    printf("Result: %s\n\n", valid1 ? "VALID" : "INVALID");
    
    printf("\n=== Test 2: MOVE|2|3 if swapped to row=3, col=2 ===\n");
    bool valid2 = is_valid_move(&game, 3, 2);
    printf("Result: %s\n\n", valid2 ? "VALID" : "INVALID");
    
    return 0;
}
