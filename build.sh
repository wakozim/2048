#!/bin/sh

set -xe

mkdir -p ./build/

CFLAGS="-O3 -Wall -Wextra -g -pedantic `pkg-config --cflags raylib`"
CLIBS="`pkg-config --libs raylib` -lm"

gcc $CFLAGS -o ./build/2048 ./src/gui-version.c ./src/2048.c $CLIBS
