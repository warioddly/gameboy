#!/bin/bash

SRC_DIR="lib/src/ffi"
OUT_DIR="macos/"
LIB_NAME="libgameboy"
EXT="dylib"
FLAG="-dynamiclib"

mkdir -p "$OUT_DIR"

echo "Compiling C files in $SRC_DIR to $OUT_DIR/$LIB_NAME.$EXT"

gcc $FLAG -o "$OUT_DIR/$LIB_NAME.$EXT" $(find "$SRC_DIR" -name '*.c')

echo "Build complete: $OUT_DIR/$LIB_NAME.$EXT"