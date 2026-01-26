#ifndef GAME_H
#define GAME_H

#include "../common/board.h"
#include <stdbool.h>

typedef enum {
    GAME_STATUS_IN_PROGRESS,
    GAME_STATUS_BLACK_WINS,
    GAME_STATUS_WHITE_WINS,
    GAME_STATUS_DRAW
} GameStatus;

typedef enum {
    PLAYER_BLACK,
    PLAYER_WHITE
} Player;

typedef struct {
    char board[BOARD_HEIGHT][BOARD_WIDTH];
    Player current_player;
    GameStatus status;
    bool black_can_move;
    bool white_can_move;
} GameState;

void initialize_game(GameState *game);
bool is_valid_move(const GameState *game, int row, int col);
bool execute_move(GameState *game, int row, int col);
bool has_legal_moves(const GameState *game, Player player);
bool is_game_over(const GameState *game);
void count_pieces(const GameState *game, int *black_count, int *white_count);
GameStatus determine_winner(const GameState *game);

#endif
