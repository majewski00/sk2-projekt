#include "game.h"
#include <string.h>

static const int DIRECTIONS[8][2] = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
    {1, 0}, {1, -1}, {0, -1}, {-1, -1}
};

static char get_player_cell(Player player) {
    return (player == PLAYER_BLACK) ? CELL_BLACK : CELL_WHITE;
}

static char get_opponent_cell(Player player) {
    return (player == PLAYER_BLACK) ? CELL_WHITE : CELL_BLACK;
}

static bool is_within_bounds(int row, int col) {
    return row >= 0 && row < BOARD_HEIGHT && col >= 0 && col < BOARD_WIDTH;
}

static int check_direction_for_flip(const GameState *game, int row, int col, int dir_row, int dir_col) {
    char player_cell = get_player_cell(game->current_player);
    char opponent_cell = get_opponent_cell(game->current_player);
    
    int current_row = row + dir_row;
    int current_col = col + dir_col;
    int count = 0;
    
    while (is_within_bounds(current_row, current_col)) {
        char cell = game->board[current_row][current_col];
        
        if (cell == CELL_EMPTY) {
            return 0;
        }
        
        if (cell == opponent_cell) {
            count++;
            current_row += dir_row;
            current_col += dir_col;
            continue;
        }
        
        if (cell == player_cell) {
            return count;
        }
        
        return 0;
    }
    
    return 0;
}

static void flip_pieces_in_direction(GameState *game, int row, int col, int dir_row, int dir_col, int count) {
    char player_cell = get_player_cell(game->current_player);
    
    int current_row = row + dir_row;
    int current_col = col + dir_col;
    
    for (int i = 0; i < count; i++) {
        game->board[current_row][current_col] = player_cell;
        current_row += dir_row;
        current_col += dir_col;
    }
}

void initialize_game(GameState *game) {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            game->board[row][col] = CELL_EMPTY;
        }
    }
    
    game->board[3][3] = CELL_WHITE;
    game->board[3][4] = CELL_BLACK;
    game->board[4][3] = CELL_BLACK;
    game->board[4][4] = CELL_WHITE;
    
    game->current_player = PLAYER_BLACK;
    game->status = GAME_STATUS_IN_PROGRESS;
    game->black_can_move = true;
    game->white_can_move = true;
}

void initialize_test_game(GameState *game) {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            game->board[row][col] = CELL_BLACK;
        }
    }
    
    game->board[0][7] = CELL_EMPTY;
    game->board[1][7] = CELL_WHITE;
    game->board[2][7] = CELL_WHITE;
    game->board[3][7] = CELL_WHITE;
    game->board[4][7] = CELL_WHITE;
    game->board[5][7] = CELL_WHITE;
    game->board[6][7] = CELL_WHITE;
    game->board[7][7] = CELL_WHITE;
    game->board[7][6] = CELL_WHITE;
    game->board[7][5] = CELL_WHITE;
    
    game->current_player = PLAYER_BLACK;
    game->status = GAME_STATUS_IN_PROGRESS;
    game->black_can_move = true;
    game->white_can_move = true;
}

bool is_valid_move(const GameState *game, int row, int col) {
    if (!is_within_bounds(row, col)) {
        return false;
    }
    
    if (game->board[row][col] != CELL_EMPTY) {
        return false;
    }
    
    for (int i = 0; i < 8; i++) {
        int dir_row = DIRECTIONS[i][0];
        int dir_col = DIRECTIONS[i][1];
        
        if (check_direction_for_flip(game, row, col, dir_row, dir_col) > 0) {
            return true;
        }
    }
    
    return false;
}

bool execute_move(GameState *game, int row, int col) {
    if (!is_valid_move(game, row, col)) {
        return false;
    }
    
    char player_cell = get_player_cell(game->current_player);
    game->board[row][col] = player_cell;
    
    for (int i = 0; i < 8; i++) {
        int dir_row = DIRECTIONS[i][0];
        int dir_col = DIRECTIONS[i][1];
        int flip_count = check_direction_for_flip(game, row, col, dir_row, dir_col);
        
        if (flip_count > 0) {
            flip_pieces_in_direction(game, row, col, dir_row, dir_col, flip_count);
        }
    }
    
    game->current_player = (game->current_player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
    
    game->black_can_move = has_legal_moves(game, PLAYER_BLACK);
    game->white_can_move = has_legal_moves(game, PLAYER_WHITE);
    
    if (is_game_over(game)) {
        game->status = determine_winner(game);
    }
    
    return true;
}

bool has_legal_moves(const GameState *game, Player player) {
    GameState temp_game = *game;
    temp_game.current_player = player;
    
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (is_valid_move(&temp_game, row, col)) {
                return true;
            }
        }
    }
    
    return false;
}

bool is_game_over(const GameState *game) {
    return !game->black_can_move && !game->white_can_move;
}

void count_pieces(const GameState *game, int *black_count, int *white_count) {
    *black_count = 0;
    *white_count = 0;
    
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (game->board[row][col] == CELL_BLACK) {
                (*black_count)++;
            } else if (game->board[row][col] == CELL_WHITE) {
                (*white_count)++;
            }
        }
    }
}

GameStatus determine_winner(const GameState *game) {
    int black_count, white_count;
    count_pieces(game, &black_count, &white_count);
    
    if (black_count > white_count) {
        return GAME_STATUS_BLACK_WINS;
    } else if (white_count > black_count) {
        return GAME_STATUS_WHITE_WINS;
    } else {
        return GAME_STATUS_DRAW;
    }
}
