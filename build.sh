#!/bin/sh

set -xe

mkdir -p ./build/
mkdir -p ./wasm/

CFLAGS="-O3 -Wall -Wextra -g -pedantic `pkg-config --cflags raylib`"
CLIBS="`pkg-config --libs raylib` -lm"

clang $CFLAGS -o ./build/2048 ./src/gui-version.c ./src/2048.c $CLIBS
clang --target=wasm32 -I./include/ --no-standard-libraries -Wl,--export-table -Wl,--no-entry -Wl,--allow-undefined -Wl,--export=main -Wl,--export=__head_base -Wl,--allow-undefined -o ./wasm/2048.wasm ./src/gui-version.c ./src/2048.c -DPLATFORM_WEB
