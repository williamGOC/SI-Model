#!/bin/bash
# =======================================================
# Compilation and execution script with custom lambda values
# =======================================================

# System parameters (with default values)
PHI=${1:-0.9}           # Particle density
RC=${2:-2.5}            # Cutoff radius for interactions
N=${3:-1000}            # Number of particles
ALPHA=${4:-1.0}         # OU process relaxation rate
SIGMA=${5:-0.5}         # OU noise
DT=${6:-0.01}           # Time step
BETA=${7:-0.3}          # Recovery rate (I -> S)
REALIZ=${8:-50}         # Number of realizations per lambda

# Array de valores de lambda (especificar manualmente aquí)
LAMBDAS=(0.5 1.0 1.5 2.0)

# Compiler settings
GCC=gcc
LDFLAGS="-lm"

# Source files and output
SRC="main.c src/system.c src/random.c"
OUT="simulation"

# Output base directory
OUTPUT_BASE="SERIE"

# Create output base directory if it doesn't exist
mkdir -p "$OUTPUT_BASE"

# Display compilation parameters
echo "# =========================================="
echo "# Compilation and custom lambda sweep:"
echo "# PHI              = ${PHI}    (density)"
echo "# RC               = ${RC}     (cutoff radius)"
echo "# N                = ${N}      (particles)"
echo "# ALPHA            = ${ALPHA}  (OU relaxation)"
echo "# SIGMA            = ${SIGMA}  (OU noise)"
echo "# DT               = ${DT}     (time step)"
echo "# BETA             = ${BETA}   (recovery rate)"
echo "# REALIZ           = ${REALIZ} (realizations per lambda)"
echo "# LAMBDAS          = ${LAMBDAS[@]}"
echo "# OUTPUT_BASE      = ${OUTPUT_BASE}"
echo "# =========================================="
echo ""

# Loop over custom lambda values
for LAMBDA in "${LAMBDAS[@]}"; do
    # Format lambda with consistent decimal places
    LAMBDA_FORMATTED=$(printf "%.4f" $LAMBDA)
    
    # Create subdirectory for this lambda value
    LAMBDA_DIR="${OUTPUT_BASE}/lambda_${LAMBDA_FORMATTED}"
    mkdir -p "$LAMBDA_DIR"
    
    echo "# =========================================="
    echo "# LAMBDA = ${LAMBDA_FORMATTED}"
    echo "# Output directory: ${LAMBDA_DIR}"
    echo "# =========================================="
    
    # Run realizations
    for ((r=0; r < REALIZ; r++)); do
        # Create output file for this realization
        OUTPUT_FILE="${LAMBDA_DIR}/lambda_${LAMBDA_FORMATTED}_real_${r}.dat"
        
        # Compile with current lambda value
        CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N} -DALPHA=${ALPHA} -DSIGMA=${SIGMA} -DDT=${DT} -DBETA=${BETA} -DLAMBDA=${LAMBDA}"
        
        if [ $r -eq 0 ]; then
            echo "# Compiling..."
            $GCC $CFLAGS $SRC $LDFLAGS -o $OUT 2>&1 | grep -i "error" && {
                echo "# ❌ Compilation failed for LAMBDA=${LAMBDA_FORMATTED}!"
                exit 1
            }
        fi
        
        # Run simulation and save results
        echo "# Realization $((r+1))/${REALIZ}..."
        ./$OUT > "$OUTPUT_FILE"
        
        # Show progress
        FILE_SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)
        echo "# ✓ Saved: ${OUTPUT_FILE} (${FILE_SIZE})"
    done
    
    echo ""
done

echo "# =========================================="
echo "# ✓ All simulations completed!"
echo "# Results saved in: ${OUTPUT_BASE}/"
echo "# =========================================="

# List generated directories and files
echo ""
echo "# Directory structure:"
find "$OUTPUT_BASE" -type d | sort

echo ""
echo "# Total files generated:"
find "$OUTPUT_BASE" -type f -name "*.dat" | wc -l | awk '{print "# " $1 " files"}'