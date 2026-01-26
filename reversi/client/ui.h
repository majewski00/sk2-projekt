#ifndef UI_H
#define UI_H

void display_board(const char *board_string);
void display_status(const char *message);
void display_welcome(const char *color);
void display_waiting(void);
int get_player_move(int *row, int *col);
int parse_move_input(const char *input, int *row, int *col);
void display_error(const char *message);
void display_opponent_move(int row, int col);
void display_game_over(const char *result, const char *winner, int black_count, int white_count);

#endif
