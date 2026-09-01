#!/usr/bin/env bash
# ==============================================================================
# dump_trace.sh
# Dumps the Percepio Trace snapshot buffer from STM32H7 RAM into trace.bin
# ==============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_FILE="${1:-$SCRIPT_DIR/trace.bin}"

echo "Capturing FreeRTOS trace snapshot from STM32H7..."
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
    -c "init; halt; dump_image $OUTPUT_FILE 0x2000572c 0x35f8; resume; exit"

echo "Trace saved to: $OUTPUT_FILE"
echo "You can now open $OUTPUT_FILE in Percepio View / Tracealyzer."
