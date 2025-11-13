#!/bin/bash

# =======================================================
# Compilation script for simple main (no OpenGL)
# =======================================================

# System parameters (with default values)
PHI=${1:-0.9}      # Particle density
RC=${2:-2.5}       # Cutoff radius for interactions
N=${3:-1000}       # Number of particles
ALPHA=${4:-1.0}    # OU process relaxation rate
SIGMA=${5:-0.01}    # OU process noise strength
DT=${6:-0.01}      # Time step
BETA=${7:-1.0}     # Recovery rate (I -> S)
LAMBDA=${8:-1.0}   # Spatial decay of infection
REALIZ=${9:-1000}  # Number of realizations per sigma

# Compiler settings
GCC=gcc
CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N} -DALPHA=${ALPHA} -DSIGMA=${SIGMA} -DDT=${DT} -DBETA=${BETA} -DLAMBDA=${LAMBDA} -DREALIZATION=${REALIZ}"
LDFLAGS="-lm"

# Source files and output
SRC="meassure.c src/system.c src/random.c"
OUT="meassure"

# Display compilation parameters
echo "# =========================================="
echo "# Compilation parameters:"
echo "# PHI         = ${PHI}    (density)"
echo "# RC          = ${RC}     (cutoff radius)"
echo "# N           = ${N}      (particles)"
echo "# ALPHA       = ${ALPHA}  (OU relaxation)"
echo "# SIGMA       = ${SIGMA}  (OU noise)"
echo "# DT          = ${DT}     (time step)"
echo "# BETA        = ${BETA}   (recovery rate)"
echo "# LAMBDA      = ${LAMBDA} (infection decay)"
echo "# REALIZATION = ${REALIZ} (realization)"
echo "# =========================================="
echo ""

# Compile
echo "$GCC $CFLAGS $SRC $LDFLAGS -o $OUT"
$GCC $CFLAGS $SRC $LDFLAGS -o $OUT

# Check compilation result
if [ $? -eq 0 ]; then
    echo ""
    echo "# =========================================="
    echo "# Compilation successful!"
    echo "# Run with: ./$OUT"
    echo "# Redirect data: ./$OUT | grep -v '^#' > data.txt"
    echo "# =========================================="
else
    echo ""
    echo "# =========================================="
    echo "# Compilation failed!"
    echo "# =========================================="
    exit 1
fi