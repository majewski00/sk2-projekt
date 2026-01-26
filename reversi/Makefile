CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SERVER_SRC = server/main.c server/network.c server/matchmaking.c server/game.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_BIN = server_bin

CLIENT_SRC = client/main.c client/network.c client/ui.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_BIN = client_bin

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

server/main.o: server/main.c server/server.h server/network.h server/matchmaking.h
	$(CC) $(CFLAGS) -c $< -o $@

server/network.o: server/network.c server/network.h server/game.h common/protocol.h common/board.h
	$(CC) $(CFLAGS) -c $< -o $@

server/matchmaking.o: server/matchmaking.c server/matchmaking.h server/network.h server/game.h common/protocol.h
	$(CC) $(CFLAGS) -c $< -o $@

server/game.o: server/game.c server/game.h common/board.h
	$(CC) $(CFLAGS) -c $< -o $@

client/main.o: client/main.c client/client.h client/network.h client/ui.h common/protocol.h
	$(CC) $(CFLAGS) -c $< -o $@

client/network.o: client/network.c client/network.h common/protocol.h
	$(CC) $(CFLAGS) -c $< -o $@

client/ui.o: client/ui.c client/ui.h common/board.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJ) $(SERVER_BIN) $(CLIENT_OBJ) $(CLIENT_BIN)

.PHONY: all clean
