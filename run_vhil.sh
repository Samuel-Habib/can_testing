#!/usr/bin/env bash

# Exit on error
set -e

# Parse arguments to decide mode: normal (auto-continue), step (halted), or no-gdb
MODE="normal"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --step|-s|step)
            MODE="step"
            shift
            ;;
        --no-gdb|-n|no-gdb)
            MODE="no-gdb"
            shift
            ;;
        --normal|-r|normal)
            MODE="normal"
            shift
            ;;
        --help|-h|help)
            echo "Usage: $0 [--normal | --step | --no-gdb]"
            echo "  --normal, -r, normal : Run normally (GDB auto-continues) [Default]"
            echo "  --step,   -s, step   : Step through (GDB halts at startup)"
            echo "  --no-gdb, -n, no-gdb : Run without debugging/GDB client"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--normal | --step | --no-gdb]"
            exit 1
            ;;
    esac
done

echo "Running in mode: $MODE"

# 1. Setup virtual CAN if not already present
if ! ip link show vcan0 >/dev/null 2>&1; then
    echo "vcan0 not found. Setting up virtual CAN interface (may require sudo)..."
    sudo modprobe vcan
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
fi

# 2. Cleanup old PTY symbolic link if it exists
rm -f /tmp/uart

# Ensure logs directory exists
mkdir -p logs

# 3. Detect if already inside a tmux session
if [ -n "$TMUX" ]; then
    echo "Running inside an active tmux session. Setting up Dev window and splitting VHIL..."
    
    # Capture original window so we can return to it
    ORIGINAL_WINDOW=$(tmux display-message -p '#I')
    
    # 3a. Setup Dev window for commands, logs, and agy terminal
    tmux new-window -n "Dev"
    
    # Split Dev window horizontally (creates right pane for agy)
    tmux split-window -h
    tmux send-keys "agy" C-m
    
    # Move back to left pane, split vertically for log tailing
    tmux select-pane -L
    tmux split-window -v
    tmux send-keys "tail -f logs/renode.log" C-m
    
    # Focus back up to Dev window general command runner (top-left pane)
    tmux select-pane -U
    
    # 3b. Go back to the original window to set up the HIL layout
    tmux select-window -t "$ORIGINAL_WINDOW"
    
    # Split vertically (creates right column)
    tmux split-window -h
    # In the right column, wait for /tmp/uart, then open screen in a persistent loop
    tmux send-keys "t=10; while [ ! -e /tmp/uart ] && [ \$t -gt 0 ]; do sleep 0.5; ((t--)); done; if [ -e /tmp/uart ]; then while true; do screen /tmp/uart; sleep 1; done; else echo 'Error: /tmp/uart PTY creation timed out.'; fi" C-m
    
    # Split right column vertically (creates bottom-right pane)
    tmux split-window -v
    # In the bottom-right pane, run candump
    tmux send-keys "candump vcan0" C-m
    
    # Return focus to the original left column
    tmux select-pane -L
    
    if [ "$MODE" != "no-gdb" ]; then
        GDB_CMD="sleep 1.5 && gdb-multiarch -q -x gdb_init.gdb -ex 'target remote localhost:3333' -ex 'load' -ex 'layout split' -ex 'focus cmd'"
        if [ "$MODE" = "normal" ]; then
            GDB_CMD="$GDB_CMD -ex 'continue'"
        fi
        GDB_CMD="$GDB_CMD build/ru-tel.elf"

        # Split the left column vertically (creates left-bottom pane)
        tmux split-window -v
        tmux send-keys "$GDB_CMD" C-m
        # Move focus back up to the Renode Monitor pane
        tmux select-pane -U
    fi
    
    # Execute Renode directly in the original pane (replacing the shell process)
    exec renode --disable-xwt --console setup.resc
else
    # We are not in tmux. Start a new detached session.
    SESSION_NAME="renode-vhil"
    tmux kill-session -t "$SESSION_NAME" 2>/dev/null || true
    
    # Start detached session running Renode Monitor in the first window "VHIL"
    tmux new-session -d -s "$SESSION_NAME" -n "VHIL" "renode --disable-xwt --console setup.resc"
    
    echo "Waiting for Renode to initialize and create /tmp/uart..."
    for i in {1..10}; do
        if [ -L /tmp/uart ] || [ -S /tmp/uart ] || [ -e /tmp/uart ]; then
            break
        fi
        sleep 0.5
    done
    
    # Split the window vertically (creates right column)
    tmux split-window -h -t "$SESSION_NAME:VHIL"
    tmux send-keys -t "$SESSION_NAME:VHIL" "while true; do if [ -e /tmp/uart ]; then screen /tmp/uart; fi; sleep 1; done" C-m
    
    # Split right column vertically (creates bottom-right pane)
    tmux split-window -v -t "$SESSION_NAME:VHIL"
    tmux send-keys -t "$SESSION_NAME:VHIL" "candump vcan0" C-m
    
    # Move focus left back to the left column
    tmux select-pane -L -t "$SESSION_NAME:VHIL"
    
    if [ "$MODE" != "no-gdb" ]; then
        GDB_CMD="sleep 1.5 && gdb-multiarch -q -x gdb_init.gdb -ex 'target remote localhost:3333' -ex 'load' -ex 'layout split' -ex 'focus cmd'"
        if [ "$MODE" = "normal" ]; then
            GDB_CMD="$GDB_CMD -ex 'continue'"
        fi
        GDB_CMD="$GDB_CMD build/ru-tel.elf"

        # Split left column vertically (creates left-bottom pane)
        tmux split-window -v -t "$SESSION_NAME:VHIL"
        tmux send-keys -t "$SESSION_NAME:VHIL" "$GDB_CMD" C-m
        # Select the left-top pane (Renode Monitor) so focus is on Renode
        tmux select-pane -U -t "$SESSION_NAME:VHIL"
    fi
    
    # Create the second window "Dev"
    tmux new-window -t "$SESSION_NAME" -n "Dev"
    
    # Split Dev window horizontally (creates right pane for agy)
    tmux split-window -h -t "$SESSION_NAME:Dev"
    tmux send-keys -t "$SESSION_NAME:Dev" "agy" C-m
    
    # Select left pane of Dev window, split vertically for log tailing
    tmux select-pane -L -t "$SESSION_NAME:Dev"
    tmux split-window -v -t "$SESSION_NAME:Dev"
    tmux send-keys -t "$SESSION_NAME:Dev" "tail -f logs/renode.log" C-m
    
    # Select Dev window command runner (top-left pane)
    tmux select-pane -U -t "$SESSION_NAME:Dev"
    
    # Start the session in the VHIL window
    tmux select-window -t "$SESSION_NAME:VHIL"
    # Attach to the tmux session
    tmux attach-session -t "$SESSION_NAME"
    
    # Kill the session when detached
    tmux kill-session -t "$SESSION_NAME"
fi
