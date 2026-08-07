#!/usr/bin/env bash
# SRL Toolchain v0.3.2 Automated Installer for Linux & macOS
set -e

echo "========================================================"
echo "  SRL (Serial Run Language) v0.3.2 POSIX Installer"
echo "========================================================"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# 1. Build
echo "[1/4] Building SRL binaries with CMake..."
mkdir -p build
cd build
cmake ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
cd "$SCRIPT_DIR"

# 2. Installation Directories
INSTALL_DIR="$HOME/.srl"
BIN_DIR="$INSTALL_DIR/bin"
STD_DIR="$INSTALL_DIR/std"
INC_DIR="$INSTALL_DIR/include"

echo "[2/4] Deploying to '$INSTALL_DIR'..."
mkdir -p "$BIN_DIR" "$STD_DIR" "$INC_DIR"

if [ -f "$SCRIPT_DIR/build/srl" ]; then
    cp "$SCRIPT_DIR/build/srl" "$BIN_DIR/srl"
elif [ -f "$SCRIPT_DIR/build/Release/srl" ]; then
    cp "$SCRIPT_DIR/build/Release/srl" "$BIN_DIR/srl"
fi
chmod +x "$BIN_DIR/srl"

if [ -d "$SCRIPT_DIR/std" ]; then
    cp -r "$SCRIPT_DIR/std/"* "$STD_DIR/" 2>/dev/null || true
fi
if [ -d "$SCRIPT_DIR/include" ]; then
    cp -r "$SCRIPT_DIR/include/"* "$INC_DIR/" 2>/dev/null || true
fi

# 3. Environment PATH configuration
echo "[3/4] Configuring environment PATH..."
SHELL_RC=""
if [ -n "$BASH_VERSION" ]; then
    SHELL_RC="$HOME/.bashrc"
elif [ -n "$ZSH_VERSION" ]; then
    SHELL_RC="$HOME/.zshrc"
elif [ -f "$HOME/.bashrc" ]; then
    SHELL_RC="$HOME/.bashrc"
elif [ -f "$HOME/.zshrc" ]; then
    SHELL_RC="$HOME/.zshrc"
fi

if [ -n "$SHELL_RC" ]; then
    if ! grep -q "$BIN_DIR" "$SHELL_RC"; then
        echo "export PATH=\"$BIN_DIR:\$PATH\"" >> "$SHELL_RC"
        echo "[OK] Added '$BIN_DIR' to $SHELL_RC"
    fi
fi

# 4. Verification
echo "[4/4] Verifying installation..."
export PATH="$BIN_DIR:$PATH"
"$BIN_DIR/srl" version

echo "========================================================"
echo "  SRL v0.3.2 Installed Successfully to $INSTALL_DIR"
echo "========================================================"
