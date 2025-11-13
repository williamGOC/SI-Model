#!/bin/bash

# System parameters
PHI=${1:-0.9}          # Density (not used yet, but kept for compatibility)
RC=${2:-2.5}           # Cutoff radius
N=${3:-1000}           # Number of particles


# Compiler settings
GCC=gcc
CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N}"
LDFLAGS="-lm"

# Source files
SRC="main.c src/system.c src/random.c"
OUT="main"

# Display compilation parameters
echo "# =========================================="
echo "# Compilation parameters:"
echo "# PHI    = ${PHI}"
echo "# RC     = ${RC}"
echo "# N      = ${N}"
echo "# =========================================="
echo ""

# Compile
echo "# Compiling..."
echo "$GCC $CFLAGS $SRC $LDFLAGS -o $OUT"
$GCC $CFLAGS $SRC $LDFLAGS -o $OUT

# Check compilation result
if [ $? -eq 0 ]; then
    echo ""
    echo "# =========================================="
    echo "# Compilation successful!"
    echo "# Run with: ./$OUT"
    echo "# =========================================="
else
    echo ""
    echo "# =========================================="
    echo "# Compilation failed!"
    echo "# =========================================="
    exit 1
fi