#!/usr/bin/env bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
if [ -f "$SCRIPT_DIR/build/Release/srl" ]; then
    "$SCRIPT_DIR/build/Release/srl" "$@"
elif [ -f "$SCRIPT_DIR/build/srl" ]; then
    "$SCRIPT_DIR/build/srl" "$@"
else
    echo "[SRL Error] SRL engine executable not found. Please build using cmake first."
    exit 1
fi
