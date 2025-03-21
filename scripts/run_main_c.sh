#!/bin/bash

SRC_DIR="lib/src/ffi/"
BUILD_DIR="build"
OUTPUT="$BUILD_DIR/main"

CFLAGS="-Wall -Werror -std=c11 -O2"

SRC_FILES=$(find "$SRC_DIR" -name "*.c")

mkdir -p "$BUILD_DIR"

echo "🔧 Compiling..."
gcc $CFLAGS $SRC_FILES -o "$OUTPUT"

if [ $? -ne 0 ]; then
  echo "Compilation failed"
  exit 1
fi

echo "Run:"
"$OUTPUT"