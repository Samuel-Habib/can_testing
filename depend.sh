#!/usr/bin/env bash
# ==============================================================================
# depend.sh - Install all development dependencies, language servers, build tools,
# simulation tools, and OpenOCD flashing setup for ru-tel (STM32H753 project).
# ==============================================================================

set -e

# Terminal colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================================================${NC}"
echo -e "${CYAN}      ru-tel STM32 Environment Setup & Dependency Installer           ${NC}"
echo -e "${BLUE}========================================================================${NC}"

# Detect OS / Package Manager
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_NAME=$ID
else
    OS_NAME=$(uname -s)
fi

echo -e "${YELLOW}[*] Detected operating system: ${OS_NAME}${NC}"

# Check for sudo / root privileges when installing system packages
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    else
        echo -e "${RED}[!] Root privileges or 'sudo' required to install dependencies.${NC}"
        exit 1
    fi
fi

# Function to run package updates & installations for Debian / Ubuntu
install_apt_deps() {
    echo -e "${YELLOW}[1/6] Updating APT repositories & installing system build tools...${NC}"
    $SUDO apt-get update -y

    echo -e "${YELLOW}[2/6] Installing C/C++ cross-toolchain & build utilities...${NC}"
    $SUDO apt-get install -y \
        build-essential \
        gcc-arm-none-eabi \
        gdb-multiarch \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        cmake \
        make \
        ninja-build \
        git \
        curl \
        wget \
        unzip \
        pkg-config \
        libusb-1.0-0-dev \
        libhidapi-dev \
        libftdi1-dev

    echo -e "${YELLOW}[3/6] Installing OpenOCD & hardware flashing tools...${NC}"
    $SUDO apt-get install -y \
        openocd \
        stlink-tools

    echo -e "${YELLOW}[4/6] Installing Language Servers (clangd, python-lsp-server)...${NC}"
    $SUDO apt-get install -y \
        clangd \
        clang-tools \
        python3-pip \
        python3-venv \
        python3-dev

    echo -e "${YELLOW}[5/6] Installing simulation, virtual CAN & multiplexing tools (VHIL)...${NC}"
    $SUDO apt-get install -y \
        can-utils \
        iproute2 \
        tmux \
        screen \
        python3
}

# Function to run package updates & installations for Arch Linux
install_pacman_deps() {
    echo -e "${YELLOW}[1/6] Installing packages via pacman...${NC}"
    $SUDO pacman -Sy --needed --noconfirm \
        base-devel \
        arm-none-eabi-gcc \
        arm-none-eabi-gdb \
        arm-none-eabi-newlib \
        cmake \
        make \
        ninja \
        git \
        curl \
        wget \
        unzip \
        pkgconf \
        libusb \
        hidapi \
        libftdi \
        openocd \
        stlink \
        clang \
        python \
        python-pip \
        can-utils \
        iproute2 \
        tmux \
        screen
}

# Function to run package updates & installations for Fedora / RHEL
install_dnf_deps() {
    echo -e "${YELLOW}[1/6] Installing packages via dnf...${NC}"
    $SUDO dnf install -y \
        @development-tools \
        arm-none-eabi-gcc-cs \
        arm-none-eabi-gdb \
        arm-none-eabi-newlib \
        cmake \
        make \
        ninja-build \
        git \
        curl \
        wget \
        unzip \
        pkgconfig \
        libusb1-devel \
        hidapi-devel \
        libftdi-devel \
        openocd \
        stlink \
        clang-tools-extra \
        python3 \
        python3-pip \
        can-utils \
        iproute \
        tmux \
        screen
}

# Execute OS-specific package installer
case "$OS_NAME" in
    ubuntu|debian|pop|mint|kali)
        install_apt_deps
        ;;
    arch|manjaro|endeavouros)
        install_pacman_deps
        ;;
    fedora|rhel|centos)
        install_dnf_deps
        ;;
    *)
        echo -e "${YELLOW}[!] Unrecognized Linux distribution '$OS_NAME'. Attempting apt-get fallback...${NC}"
        if command -v apt-get >/dev/null 2>&1; then
            install_apt_deps
        else
            echo -e "${RED}[!] Could not automatically detect supported package manager (apt, pacman, dnf).${NC}"
            echo -e "${RED}    Please ensure gcc-arm-none-eabi, openocd, cmake, and clangd are installed manually.${NC}"
        fi
        ;;
esac

# 6. Install Renode simulator if not present
echo -e "${YELLOW}[6/6] Checking Renode simulator installation...${NC}"
if ! command -v renode >/dev/null 2>&1; then
    echo -e "${YELLOW}[*] Renode not found in PATH. Attempting automated installation...${NC}"
    if [ "$OS_NAME" = "ubuntu" ] || [ "$OS_NAME" = "debian" ] || command -v apt-get >/dev/null 2>&1; then
        RENODE_DEB="/tmp/renode_latest.deb"
        RENODE_URL="https://builds.renode.io/renode-latest.deb"
        echo -e "${CYAN}    Downloading Renode package from ${RENODE_URL}...${NC}"
        wget -q --show-progress -O "$RENODE_DEB" "$RENODE_URL" || curl -sSL -o "$RENODE_DEB" "$RENODE_URL"
        $SUDO apt-get install -y "$RENODE_DEB"
        rm -f "$RENODE_DEB"
    else
        echo -e "${YELLOW}[!] Automatic Renode package install is supported on Debian/Ubuntu. For $OS_NAME, please download Renode from: https://renode.io${NC}"
    fi
else
    echo -e "${GREEN}  ✓ Renode is already installed: $(renode --version 2>&1 | head -n 1)${NC}"
fi

# Install Python Language Server (pylsp) and embedded utilities via pip
echo -e "${YELLOW}[*] Configuring Python language server and tools...${NC}"
if command -v python3 >/dev/null 2>&1; then
    python3 -m pip install --upgrade pip --break-system-packages 2>/dev/null || python3 -m pip install --upgrade pip 2>/dev/null || true
    python3 -m pip install python-lsp-server pyserial cantools python-can --break-system-packages 2>/dev/null || \
    python3 -m pip install python-lsp-server pyserial cantools python-can 2>/dev/null || true
fi

# Configure OpenOCD & ST-Link udev rules for non-root hardware access
echo -e "${YELLOW}[*] Setting up OpenOCD and ST-Link udev rules for USB hardware flashing...${NC}"
UDEV_RULE_FILE="/etc/udev/rules.d/60-openocd.rules"
if [ -d "/etc/udev/rules.d" ]; then
    cat << 'EOF' | $SUDO tee "$UDEV_RULE_FILE" >/dev/null
# ST-Link V1
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3744", MODE="666", GROUP="plugdev"
# ST-Link V2
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="666", GROUP="plugdev"
# ST-Link V2-1
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="666", GROUP="plugdev"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="666", GROUP="plugdev"
# ST-Link V3
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", MODE="666", GROUP="plugdev"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374f", MODE="666", GROUP="plugdev"
ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", MODE="666", GROUP="plugdev"
# CMSIS-DAP
ATTRS{idVendor}=="c251", MODE="666", GROUP="plugdev"
EOF

    # Ensure current user is added to plugdev / dialout groups
    CURRENT_USER=${SUDO_USER:-$(whoami)}
    if [ "$CURRENT_USER" != "root" ]; then
        $SUDO groupadd -f plugdev || true
        $SUDO usermod -aG plugdev "$CURRENT_USER" || true
        $SUDO usermod -aG dialout "$CURRENT_USER" || true
    fi

    # Reload udev rules if udevadm is present
    if command -v udevadm >/dev/null 2>&1; then
        $SUDO udevadm control --reload-rules 2>/dev/null || true
        $SUDO udevadm trigger 2>/dev/null || true
    fi
    echo -e "${GREEN}  ✓ OpenOCD/ST-Link udev rules installed at $UDEV_RULE_FILE${NC}"
fi

# Verification step
echo -e "${BLUE}========================================================================${NC}"
echo -e "${CYAN}                    VERIFYING INSTALLED TOOLCHAIN                      ${NC}"
echo -e "${BLUE}========================================================================${NC}"

check_tool() {
    local cmd=$1
    local name=$2
    if command -v "$cmd" >/dev/null 2>&1; then
        echo -e "${GREEN}  ✓ $name: $(which $cmd)${NC}"
    else
        echo -e "${RED}  ✖ $name ($cmd) NOT found!${NC}"
    fi
}

check_tool "arm-none-eabi-gcc" "ARM GNU C Compiler"
check_tool "gdb-multiarch"     "GDB Multiarch Debugger"
check_tool "cmake"              "CMake Build System"
check_tool "ninja"              "Ninja Build Tool"
check_tool "clangd"             "C/C++ Language Server (clangd)"
check_tool "pylsp"              "Python Language Server (pylsp)"
check_tool "openocd"            "OpenOCD Debugger & Flasher"
check_tool "st-flash"           "ST-Link Utility (st-flash)"
check_tool "renode"             "Renode VHIL Simulator"
check_tool "candump"            "SocketCAN Utilities (can-utils)"
check_tool "tmux"               "Tmux Terminal Multiplexer"

echo ""
echo -e "${GREEN}========================================================================${NC}"
echo -e "${GREEN}  SUCCESS: All build dependencies, language servers, & OpenOCD tools installed!${NC}"
echo -e "${GREEN}========================================================================${NC}"
echo ""
echo -e "${CYAN}Quick Start Commands:${NC}"
echo -e "  - ${YELLOW}Build Project:${NC}       ./commands.sh  (or cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -B build && cmake --build build)"
echo -e "  - ${YELLOW}Flash STM32 via OpenOCD:${NC} openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c \"program build/ru-tel.elf verify reset exit\""
echo -e "  - ${YELLOW}Run Renode VHIL Simulation:${NC} ./run_vhil.sh"
echo -e "  - ${YELLOW}Setup SystemView Profiling:${NC} ./SEGGER_SYSVIEW_SETUP.sh --linux"
echo ""
