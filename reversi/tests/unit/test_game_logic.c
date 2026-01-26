#include <stdio.h>
#include <assert.h>
#include "server/game.h"

void print_board(const GameState *game) {
    printf("\n  0 1 2 3 4 5 6 7\n");
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        printf("%d ", row);
        for (int col = 0; col < BOARD_WIDTH; col++) {
            printf("%c ", game->board[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

void test_initial_setup(void) {
    printf("Testing initial setup...\n");
    GameState game;
    initialize_game(&game);
    
    assert(game.board[3][3] == CELL_WHITE);
    assert(game.board[3][4] == CELL_BLACK);
    assert(game.board[4][3] == CELL_BLACK);
    assert(game.board[4][4] == CELL_WHITE);
    assert(game.current_player == PLAYER_BLACK);
    assert(game.status == GAME_STATUS_IN_PROGRESS);
    
    int black_count, white_count;
    count_pieces(&game, &black_count, &white_count);
    assert(black_count == 2);
    assert(white_count == 2);
    
    printf("Initial setup: PASS\n");
}

void test_valid_moves(void) {
    printf("Testing valid moves...\n");
    GameState game;
    initialize_game(&game);
    
    assert(is_valid_move(&game, 2, 3) == true);
    assert(is_valid_move(&game, 3, 2) == true);
    assert(is_valid_move(&game, 4, 5) == true);
    assert(is_valid_move(&game, 5, 4) == true);
    
    assert(is_valid_move(&game, 0, 0) == false);
    assert(is_valid_move(&game, 3, 3) == false);
    assert(is_valid_move(&game, 7, 7) == false);
    
    printf("Valid moves: PASS\n");
}

void test_execute_move(void) {
    printf("Testing execute move...\n");
    GameState game;
    initialize_game(&game);
    
    printf("Initial board:");
    print_board(&game);
    
    assert(execute_move(&game, 2, 3) == true);
    printf("After BLACK plays (2,3):");
    print_board(&game);
    
    assert(game.board[2][3] == CELL_BLACK);
    assert(game.board[3][3] == CELL_BLACK);
    assert(game.current_player == PLAYER_WHITE);
    
    int black_count, white_count;
    count_pieces(&game, &black_count, &white_count);
    assert(black_count == 4);
    assert(white_count == 1);
    
    printf("Execute move: PASS\n");
}

void test_multiple_moves(void) {
    printf("Testing multiple moves...\n");
    GameState game;
    initialize_game(&game);
    
    assert(execute_move(&game, 2, 3) == true);
    assert(game.current_player == PLAYER_WHITE);
    printf("After BLACK plays (2,3):");
    print_board(&game);
    
    assert(execute_move(&game, 2, 2) == true);
    assert(game.current_player == PLAYER_BLACK);
    printf("After WHITE plays (2,2):");
    print_board(&game);
    
    assert(execute_move(&game, 3, 2) == true);
    assert(game.current_player == PLAYER_WHITE);
    printf("After BLACK plays (3,2):");
    print_board(&game);
    
    printf("Multiple moves: PASS\n");
}

void test_has_legal_moves(void) {
    printf("Testing legal moves check...\n");
    GameState game;
    initialize_game(&game);
    
    assert(has_legal_moves(&game, PLAYER_BLACK) == true);
    assert(has_legal_moves(&game, PLAYER_WHITE) == true);
    
    printf("Legal moves check: PASS\n");
}

int main(void) {
    printf("=== Running Game Logic Tests ===\n\n");
    
    test_initial_setup();
    test_valid_moves();
    test_execute_move();
    test_multiple_moves();
    test_has_legal_moves();
    
    printf("\n=== All Tests Passed! ===\n");
    return 0;
}
