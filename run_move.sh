#!/bin/bash

# =======================================================
# Compilation script for SIS epidemic simulation
# =======================================================

# System parameters (with default values)
PHI=${1:-0.9}      # Particle density
RC=${2:-2.5}       # Cutoff radius for interactions
N=${3:-1000}       # Number of particles
ALPHA=${4:-1.0}    # OU process relaxation rate
SIGMA=${5:-0.5}    # OU process noise strength
DT=${6:-0.01}      # Time step
BETA=${7:-0.6}     # Recovery rate (I -> S)
LAMBDA=${8:-2.0}   # Spatial decay of infection

# Compiler settings
GCC=gcc
CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N} -DALPHA=${ALPHA} -DSIGMA=${SIGMA} -DDT=${DT} -DBETA=${BETA} -DLAMBDA=${LAMBDA}"
LDFLAGS="-lGL -lGLU -lglut -lm"

# Source files and output
SRC="move.c src/system.c src/random.c"
OUT="move"

# Display compilation parameters
echo "# =========================================="
echo "# Compilation parameters:"
echo "# PHI    = ${PHI}    (density)"
echo "# RC     = ${RC}     (cutoff radius)"
echo "# N      = ${N}      (particles)"
echo "# ALPHA  = ${ALPHA}  (OU relaxation)"
echo "# SIGMA  = ${SIGMA}  (OU noise)"
echo "# DT     = ${DT}     (time step)"
echo "# BETA   = ${BETA}   (recovery rate)"
echo "# LAMBDA = ${LAMBDA} (infection decay)"
echo "# =========================================="
echo ""
echo "$GCC $CFLAGS $SRC $LDFLAGS -o $OUT"

# Compile
$GCC $CFLAGS $SRC $LDFLAGS -o $OUT

# Check compilation result
if [ $? -eq 0 ]; then
    echo ""
    echo "# Compilation successful. Run with ./$OUT"
else
    echo ""
    echo "# Compilation error."
    exit 1
fi