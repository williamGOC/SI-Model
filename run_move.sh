#!/bin/bash

# System parameters
PHI=${1:-0.9}          
RC=${2:-2.5}         
N=${3:-1000}       

GCC=gcc
#ARCH_FLAGS="-arch=sm_86"
CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N}"
LDFLAGS="-lGL -lGLU -lglut -lm"
SRC="move.c src/system.c src/random.c"
OUT="move"

echo "Compilando con PHI=${PHI}, DRC=${RC}, N=${N}"
echo "$GCC $CFLAGS $SRC $LDFLAGS -o $OUT"

$GCC $CFLAGS $SRC $LDFLAGS -o $OUT

if [ $? -eq 0 ]; then
    echo "Compilación exitosa. Ejecuta con ./$OUT"
else
    echo "Error durante la compilación."
fi