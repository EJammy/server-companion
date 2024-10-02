#!/bin/bash

set -evx
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DPICO_BOARD=pico_w  -S . -B build
cmake --build build
picotool load -f build/main.elf
