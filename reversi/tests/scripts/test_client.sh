#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <host> <port>"
    exit 1
fi

HOST=$1
PORT=$2

# Temp file to hold the nc process ID
PIDFILE="/tmp/reversi_nc_$$"

# Function to display board
display_board() {
    local board=$1
    echo "--- Board State ---"
    echo "  0 1 2 3 4 5 6 7"
    for i in {0..7}; do
        echo -n "$i "
        for j in {0..7}; do
            pos=$((i * 8 + j))
            echo -n "${board:$pos:1} "
        done
        echo
    done
    echo "-------------------"
}

# Cleanup function
cleanup() {
    if [ -f "$PIDFILE" ]; then
        NC_PID=$(cat "$PIDFILE")
        kill $NC_PID 2>/dev/null
        rm -f "$PIDFILE"
    fi
    kill $READER_PID 2>/dev/null
}
trap cleanup EXIT

# Use exec to redirect file descriptors for bidirectional communication with nc
exec 3<>/dev/tcp/$HOST/$PORT

# Background process to read from server
{
    while IFS= read -r line <&3 2>/dev/null; do
        if [[ $line == BOARD\|* ]]; then
            board="${line#BOARD|}"
            display_board "$board"
        elif [[ $line == WELCOME\|* ]]; then
            color="${line#WELCOME|}"
            echo "*** You are playing as: $color ***"
        elif [[ $line == YOUR_TURN ]]; then
            echo ">>> Your turn! Enter move (e.g., MOVE|2|3 or PASS or QUIT):"
        else
            echo "$line"
        fi
    done
} &
READER_PID=$!

# Main loop to read user input and send to server
while true; do
    read -r user_input
    if [ -z "$user_input" ]; then
        continue
    fi
    echo "$user_input" >&3
    if [[ "$user_input" == "QUIT" ]] || [[ "$user_input" == "quit" ]]; then
        sleep 0.5
        break
    fi
done

# Close the connection
exec 3>&-
exec 3<&-
