#!/bin/bash

echo "Arduino UNO R3 AVR-8:"
echo "---------------------"

for dir in ./tests/AVR/*/; do
    echo "$(realpath "$dir")"
    mkdir -p "build/AVR/$(basename "$dir")"
    python3 main.py --project-path "$(realpath "$dir")" --output-path "build/AVR/$(basename "$dir")" --mcu arduino_uno
done

echo ""
echo "ESP32-C3 mini RISC-V32"
echo "---------------------"

for dir in ./tests/RISCV/*/; do
    echo "$(realpath "$dir")"
    mkdir -p "build/RISCV/$(basename "$dir")"
    python3 main.py --project-path "$(realpath "$dir")" --output-path "build/RISCV/$(basename "$dir")" --mcu esp32c3
done