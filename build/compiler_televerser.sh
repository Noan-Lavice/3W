#!/bin/sh
# ============================================================
#  Script de compilation / televersement - NUCLEO-L476RG
#  (Linux / macOS)  :  ./compiler_televerser.sh
# ============================================================
cd "$(dirname "$0")" || exit 1
PIO="${PIO:-$HOME/.platformio/penv/bin/pio}"

echo "=== Compilation ===";   "$PIO" run                  || exit 1
echo "=== Televersement ==="; "$PIO" run --target upload  || exit 1
echo "=== Console serie 115200 bauds (Ctrl+C pour quitter) ==="
"$PIO" device monitor
