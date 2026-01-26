#include <stdio.h>
#include "server/game.h"

int main() {
    GameState game;
    initialize_game(&game);
    
    printf("Initial board:\n");
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        printf("%d ", r);
        for (int c = 0; c < BOARD_WIDTH; c++) {
            printf("%c ", game.board[r][c]);
        }
        printf("\n");
    }
    
    printf("\n=== Testing scenario from Issue B ===\n");
    printf("Setting up board state:\n");
    printf("Row 3: . . . W W W . .\n");
    printf("Row 4: . . . B B B . .\n\n");
    
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
    
    printf("Actual board state:\n");
    printf("  0 1 2 3 4 5 6 7\n");
    for (int r = 0; r < BOARD_HEIGHT; r++) {
        printf("%d ", r);
        for (int c = 0; c < BOARD_WIDTH; c++) {
            printf("%c ", game.board[r][c]);
        }
        printf("\n");
    }
    
    printf("\n=== Testing MOVE|2|3 (row=2, col=3) ===\n");
    printf("Current player: BLACK\n");
    printf("Position (2,3) is: '%c'\n", game.board[2][3]);
    printf("Position (3,3) is: '%c' (should be W)\n", game.board[3][3]);
    printf("Position (4,3) is: '%c' (should be B)\n\n", game.board[4][3]);
    
    bool valid = is_valid_move(&game, 2, 3);
    
    printf("\n=== RESULT ===\n");
    printf("is_valid_move(2, 3) returned: %s\n", valid ? "TRUE" : "FALSE");
    printf("Expected: TRUE\n");
    
    if (!valid) {
        printf("\n!!! BUG CONFIRMED: Valid move rejected !!!\n");
        return 1;
    } else {
        printf("\n✓ Move validation working correctly\n");
        return 0;
    }
}
