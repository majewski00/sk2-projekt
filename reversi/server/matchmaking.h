#ifndef MATCHMAKING_H
#define MATCHMAKING_H

void initialize_matchmaking(void);
void add_waiting_player(int client_socket);
int has_waiting_players(void);
void handle_new_connection(int client_socket);

#endif
