#!/usr/bin/env bash

# ==============================================================================
# SEGGER_SYSVIEW_SETUP.sh
# Automates the complete SEGGER SystemView + FreeRTOS setup for STM32 targets.
# Supports Linux (--linux / -l) and macOS (--macos / -m) environments.
# ==============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SHOW_HELP=false
OS_MODE=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --linux|-l)
            OS_MODE="linux"
            shift
            ;;
        --macos|--mac|-m)
            OS_MODE="macos"
            shift
            ;;
        --help|-h)
            SHOW_HELP=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            SHOW_HELP=true
            shift
            ;;
    esac
done

if [ "$SHOW_HELP" = true ] || [ -z "$OS_MODE" ]; then
    echo "Usage: $0 [--linux | --macos]"
    echo "Automates SEGGER SystemView integration into the FreeRTOS STM32 CMake project."
    echo ""
    echo "Options:"
    echo "  --linux, -l  : Configure SystemView for Linux host"
    echo "  --macos, -m  : Configure SystemView for macOS host"
    echo "  --help,  -h  : Display this help message"
    exit 0
fi

echo -e "${BLUE}=== Starting SEGGER SystemView Setup ($OS_MODE) ===${NC}"

# Detect project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CDIR="$SCRIPT_DIR"

SEGGER_DIR="$CDIR/Middlewares/Third_Party/SEGGER"
INC_DIR="$SEGGER_DIR/Inc"
SRC_DIR="$SEGGER_DIR/Src"
TMP_DIR="/tmp/segger_sysview_setup_$$"

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

mkdir -p "$INC_DIR" "$SRC_DIR" "$TMP_DIR"

# 1. Fetch official SEGGER Target repos
echo -e "${YELLOW}[1/5] Fetching official SEGGER SystemView and RTT repositories...${NC}"
git clone --depth 1 https://github.com/SEGGERMicro/SystemView.git "$TMP_DIR/SystemView" >/dev/null 2>&1
git clone --depth 1 https://github.com/SEGGERMicro/RTT.git "$TMP_DIR/RTT" >/dev/null 2>&1

# 2. Copy Target C & Header files
echo -e "${YELLOW}[2/5] Copying SystemView & RTT source files...${NC}"
cp "$TMP_DIR/RTT/RTT"/*.h "$INC_DIR/"
cp "$TMP_DIR/RTT/RTT"/*.c "$SRC_DIR/"

cp "$TMP_DIR/SystemView/SYSVIEW"/*.h "$INC_DIR/"
cp "$TMP_DIR/SystemView/SYSVIEW"/*.c "$SRC_DIR/"

cp "$TMP_DIR/SystemView/SEGGER"/*.h "$INC_DIR/"
cp "$TMP_DIR/SystemView/Config/Global.h" "$INC_DIR/"

cp "$TMP_DIR/SystemView/Sample/FreeRTOSV10"/*.h "$INC_DIR/"
cp "$TMP_DIR/SystemView/Sample/FreeRTOSV10"/*.c "$SRC_DIR/"
cp "$TMP_DIR/SystemView/Sample/FreeRTOSV10/Config/Cortex-M/SEGGER_SYSVIEW_Config_FreeRTOS.c" "$SRC_DIR/"

# 3. Create target configuration headers
echo -e "${YELLOW}[3/5] Generating SEGGER_SYSVIEW_Conf.h and SEGGER_RTT_Conf.h...${NC}"

cat << 'EOF' > "$INC_DIR/SEGGER_SYSVIEW_Conf.h"
#ifndef SEGGER_SYSVIEW_CONF_H
#define SEGGER_SYSVIEW_CONF_H

#include "FreeRTOS.h"
#include "stm32h7xx.h"

#define SEGGER_SYSVIEW_CORE                     SEGGER_SYSVIEW_CORE_CM3
#define SEGGER_SYSVIEW_GET_INTERRUPT_ID()      ((__get_IPSR()) & 0x1FF)
#define SEGGER_SYSVIEW_GET_TIMESTAMP()         (DWT->CYCCNT)
#define SEGGER_SYSVIEW_TIMESTAMP_BITS          32

#define SEGGER_SYSVIEW_MAX_ARGUMENTS            4
#define SEGGER_SYSVIEW_MAX_COMM_BYTES          1024

#endif
EOF

cat << 'EOF' > "$INC_DIR/SEGGER_RTT_Conf.h"
#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H

#define RTT_USE_ASM                             (0)

#define SEGGER_RTT_MAX_NUM_UP_BUFFERS           2
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS         2
#define BUFFER_SIZE_UP                          1024
#define BUFFER_SIZE_DOWN                        16

#define SEGGER_RTT_MODE_DEFAULT                 SEGGER_RTT_MODE_NO_BLOCK_SKIP

#endif
EOF

# Fix header stdint dependency
if ! grep -q "<stdint.h>" "$INC_DIR/SEGGER_RTT.h"; then
    sed -i.bak '/#define SEGGER_RTT_H/a \
#include <stdint.h>\
#include <stddef.h>' "$INC_DIR/SEGGER_RTT.h"
    rm -f "$INC_DIR/SEGGER_RTT.h.bak"
fi

# 4. Patch FreeRTOSConfig.h
FREERTOS_CONF="$CDIR/Core/Inc/FreeRTOSConfig.h"
echo -e "${YELLOW}[4/5] Patching FreeRTOSConfig.h...${NC}"

if [ -f "$FREERTOS_CONF" ]; then
    if ! grep -q "SEGGER_SYSVIEW_FreeRTOS.h" "$FREERTOS_CONF"; then
        if [ "$OS_MODE" = "macos" ]; then
            sed -i '' 's|/\* USER CODE BEGIN Includes \*/|/\* USER CODE BEGIN Includes \*/\n#include "SEGGER_SYSVIEW_FreeRTOS.h"|g' "$FREERTOS_CONF"
        else
            sed -i 's|/\* USER CODE BEGIN Includes \*/|/\* USER CODE BEGIN Includes \*/\n#include "SEGGER_SYSVIEW_FreeRTOS.h"|g' "$FREERTOS_CONF"
        fi
        echo -e "${GREEN}  ✓ Patched FreeRTOSConfig.h with SystemView header inclusion${NC}"
    else
        echo -e "${GREEN}  ✓ FreeRTOSConfig.h already includes SystemView header${NC}"
    fi
fi

# 5. Patch CMakeLists.txt
CMAKE_CONF="$CDIR/cmake/stm32cubemx/CMakeLists.txt"
echo -e "${YELLOW}[5/5] Updating CMakeLists.txt build target...${NC}"

if [ -f "$CMAKE_CONF" ]; then
    if ! grep -q "SEGGER/Inc" "$CMAKE_CONF"; then
        if [ "$OS_MODE" = "macos" ]; then
            sed -i '' 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include|\${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Inc|g' "$CMAKE_CONF"
        else
            sed -i 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include|\${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Inc|g' "$CMAKE_CONF"
        fi
    fi

    if ! grep -q "SEGGER_SYSVIEW.c" "$CMAKE_CONF"; then
        if [ "$OS_MODE" = "macos" ]; then
            sed -i '' 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c|\${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_RTT.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_RTT_printf.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW_FreeRTOS.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW_Config_FreeRTOS.c|g' "$CMAKE_CONF"
        else
            sed -i 's|\${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c|\${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_RTT.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_RTT_printf.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW_FreeRTOS.c\n    \${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/SEGGER/Src/SEGGER_SYSVIEW_Config_FreeRTOS.c|g' "$CMAKE_CONF"
        fi
    fi
    echo -e "${GREEN}  ✓ CMakeLists.txt updated with SEGGER sources and include paths${NC}"
fi

echo -e "${GREEN}=== SystemView Target Setup Completed Successfully! ===${NC}"
echo ""
if [ "$OS_MODE" = "linux" ]; then
    echo -e "${YELLOW}Host GUI Application (Linux):${NC}"
    echo "  1. Download SystemView Linux App from: https://www.segger.com/downloads/systemview/"
    echo "  2. Run SystemView: ./SystemView"
    echo "  3. For Renode TCP tracing: Target -> Start Recording -> Connection: IP (localhost:19021)"
else
    echo -e "${YELLOW}Host GUI Application (macOS):${NC}"
    echo "  1. Download SystemView macOS App from: https://www.segger.com/downloads/systemview/"
    echo "  2. Open SystemView.app from /Applications"
    echo "  3. For Renode TCP tracing: Target -> Start Recording -> Connection: IP (localhost:19021)"
fi
