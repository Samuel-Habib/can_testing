#!/usr/bin/env python3
"""
Renode Execution & Peripheral Trace Viewer
Parses and streams Renode built-in trace logs (logs/trace.log and logs/renode.log) in real-time.
Designed for use in tmux panes in place of GDB during --no-gdb mode.
"""

import sys
import os
import time
import re
import argparse

# ANSI Colors for terminal output
CYAN = "\033[1;36m"
GREEN = "\033[1;32m"
YELLOW = "\033[1;33m"
RED = "\033[1;31m"
BLUE = "\033[1;34m"
MAGENTA = "\033[1;35m"
RESET = "\033[0m"
DIM = "\033[2m"
BOLD = "\033[1m"

def print_header(mode_str):
    print(f"{BOLD}{BLUE}========================================================================{RESET}", flush=True)
    print(f"{BOLD}{CYAN}      RENODE REAL-TIME TRACE MONITOR  [{mode_str.upper()} MODE]{RESET}", flush=True)
    print(f"{BOLD}{BLUE}========================================================================{RESET}", flush=True)
    print(f"{DIM} Streaming trace & peripheral logs from logs/trace.log & logs/renode.log{RESET}", flush=True)
    print(f"{DIM} Press Ctrl+C to stop trace viewer{RESET}\n", flush=True)

def stream_trace(trace_path, renode_log_path, mode="functions"):
    print_header(mode)

    print(f"{YELLOW}[*] Waiting for Renode trace output...{RESET}", flush=True)
    
    timeout = 30
    start_time = time.time()
    while not os.path.exists(trace_path) and not os.path.exists(renode_log_path):
        if time.time() - start_time > timeout:
            print(f"{RED}[!] Timed out waiting for trace log files.{RESET}", flush=True)
            print(f"{YELLOW}    Make sure Renode is running setup.resc with tracing enabled.{RESET}", flush=True)
            break
        time.sleep(0.5)

    trace_file = None
    renode_file = None
    last_trace_ino = None
    last_renode_ino = None

    print(f"{BOLD}{GREEN}--- REAL-TIME TRACE STREAM STARTED ---{RESET}\n", flush=True)

    last_function = None
    inst_count = 0
    fn_count = 0

    try:
        while True:
            # Check/reopen trace_file if rotated or created
            if os.path.exists(trace_path):
                current_ino = os.stat(trace_path).st_ino
                if trace_file is None or current_ino != last_trace_ino:
                    if trace_file:
                        trace_file.close()
                    trace_file = open(trace_path, "r", errors="replace")
                    last_trace_ino = current_ino
                    print(f"{GREEN}[+] Opened execution trace log: {trace_path}{RESET}", flush=True)

            # Check/reopen renode_log_path if rotated or created
            if os.path.exists(renode_log_path):
                current_ino = os.stat(renode_log_path).st_ino
                if renode_file is None or current_ino != last_renode_ino:
                    if renode_file:
                        renode_file.close()
                    renode_file = open(renode_log_path, "r", errors="replace")
                    last_renode_ino = current_ino
                    print(f"{GREEN}[+] Opened Renode log: {renode_log_path}{RESET}", flush=True)

            read_any = False

            # 1. Read from execution trace
            if trace_file:
                line = trace_file.readline()
                if line:
                    read_any = True
                    inst_count += 1
                    line_str = line.strip()

                    if mode == "raw":
                        print(f"{DIM}{inst_count:08d}{RESET} | {line_str}", flush=True)
                    else:
                        if "(entry)]" in line_str or "Entering function" in line_str or "Entering" in line_str:
                            symbol_info = line_str
                            if "(entry)]" in line_str:
                                match = re.search(r'\[(.*?)\]$', line_str)
                                if match:
                                    symbol_info = match.group(1)
                            elif "Entering function" in line_str:
                                symbol_info = line_str.split("Entering function")[-1].strip()
                            elif "Entering" in line_str:
                                symbol_info = line_str.split("Entering")[-1].strip()

                            if symbol_info and symbol_info != last_function:
                                last_function = symbol_info
                                fn_count += 1
                                addr = line_str.split(':')[0] if ':' in line_str else ""
                                print(f"{CYAN}➜ [{fn_count:05d}] {GREEN}EXEC:{RESET} {BOLD}{symbol_info}{RESET} {DIM}(at {addr}){RESET}", flush=True)
                        elif mode == "functions" and not any(p in line_str for p in ["pc:", "0x"]):
                            print(f"{CYAN}➜ [{fn_count:05d}] {GREEN}TRACE:{RESET} {BOLD}{line_str}{RESET}", flush=True)

            # 2. Read from Renode log
            if renode_file:
                rline = renode_file.readline()
                if rline:
                    read_any = True
                    rline_str = rline.strip()

                    if "[ERROR]" in rline_str:
                        print(f"{RED}✖ {rline_str}{RESET}", flush=True)
                    elif "[WARNING]" in rline_str:
                        print(f"{YELLOW}⚠ {rline_str}{RESET}", flush=True)
                    elif "[INFO]" in rline_str and any(p in rline_str for p in ["fdcan", "usart", "uart", "adc", "dma", "timer", "rcc", "syscfg", "pwr"]):
                        print(f"{MAGENTA}⚡ {rline_str}{RESET}", flush=True)

            if not read_any:
                time.sleep(0.05)

    except KeyboardInterrupt:
        print(f"\n{BOLD}{YELLOW}[*] Trace viewer stopped. Processed {inst_count} instructions, {fn_count} function events.{RESET}", flush=True)
    finally:
        if trace_file:
            trace_file.close()
        if renode_file:
            renode_file.close()

def main():
    parser = argparse.ArgumentParser(description="Renode Real-Time Execution and Peripheral Trace Viewer")
    parser.add_argument("--trace", default="logs/trace.log", help="Path to execution trace file")
    parser.add_argument("--renode-log", default="logs/renode.log", help="Path to Renode log file")
    parser.add_argument("--mode", choices=["functions", "raw", "renode"], default="functions",
                        help="Display mode: 'functions' (high-level function calls & peripherals), 'raw' (all instructions), 'renode' (renode log)")
    args = parser.parse_args()

    stream_trace(args.trace, args.renode_log, args.mode)

if __name__ == "__main__":
    main()
