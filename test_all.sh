#!/bin/bash

for dir in ./tests/AVR/*/; do
    echo "$(realpath "$dir")"
    mkdir -p "build/AVR/$(basename "$dir")"
    python3 main.py --project-path "$(realpath "$dir")" --output-path "build/AVR/$(basename "$dir")" --mcu arduino_uno
done


for dir in ./tests/RISCV/*/; do
    echo "$(realpath "$dir")"
    mkdir -p "build/RISCV/$(basename "$dir")"
    python3 main.py --project-path "$(realpath "$dir")" --output-path "build/RISCV/$(basename "$dir")" --mcu esp32c3
done