#!/usr/bin/env bash
set -euo pipefail

readonly PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
pristine=auto
flash=yes
for arg in "$@"; do
    case "$arg" in
        --pristine) pristine=always ;;
        --build-only) flash=no ;;
        -h|--help)
            echo 'Usage: tools/build_flash.sh [--pristine] [--build-only]'
            echo 'Pico W / RP2040; board rpi_pico/rp2040/w; cdc-acm-console.'
            exit 0 ;;
        *) echo "Unbekannte Option: $arg" >&2; exit 2 ;;
    esac
done

# Reuse an active environment; otherwise find the workspace/project venv.
if ! command -v west >/dev/null 2>&1; then
    for activate in "$PROJECT_DIR/../../.venv/bin/activate" \
                    "$PROJECT_DIR/../../venv/bin/activate" \
                    "$PROJECT_DIR/.venv/bin/activate"; do
        if [[ -f "$activate" ]]; then
            source "$activate"
            if command -v west >/dev/null 2>&1; then break; fi
        fi
    done
fi
if ! command -v west >/dev/null 2>&1; then
    echo 'west fehlt. Keine passende .venv/venv im Zephyr-Arbeitsbereich gefunden.' >&2
    echo 'Bitte die vorhandene Zephyr-Umgebung aktivieren und erneut starten.' >&2
    exit 1
fi

cd "$PROJECT_DIR"
west build -b rpi_pico/rp2040/w -S cdc-acm-console \
    "$PROJECT_DIR" -d "$PROJECT_DIR/build" -p "$pristine"

if [[ "$flash" == yes ]]; then
    "$PROJECT_DIR/tools/flash_openocd.sh"
fi
