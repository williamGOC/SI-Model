#!/bin/bash
# =======================================================
# Compilation and execution script with LOGARITHMIC sigma sweep
# =======================================================

# System parameters (with default values)
PHI=${1:-0.9}           # Particle density
RC=${2:-2.5}            # Cutoff radius for interactions
N=${3:-1000}            # Number of particles
ALPHA=${4:-1.0}         # OU process relaxation rate
SIGMA_MIN=${5:-0.1}     # Minimum sigma
SIGMA_MAX=${6:-33.0}    # Maximum sigma
N_STEPS=${7:-20}        # Number of logarithmic steps
DT=${8:-0.01}           # Time step
BETA=${9:-0.2}          # Recovery rate (I -> S)
LAMBDA=${10:-2.0}       # Spatial decay of infection
REALIZ=${11:-1000}      # Number of realizations per sigma

# Compiler settings
GCC=gcc
LDFLAGS="-lm"

# Source files and output
SRC="meassure.c src/system.c src/random.c"
OUT="meassure"

# Output base directory
OUTPUT_BASE="R0"

# Create output base directory if it doesn't exist
mkdir -p "$OUTPUT_BASE"

# Display compilation parameters
echo "# =========================================="
echo "# Compilation and LOGARITHMIC sweep:"
echo "# PHI              = ${PHI}    (density)"
echo "# RC               = ${RC}     (cutoff radius)"
echo "# N                = ${N}      (particles)"
echo "# ALPHA            = ${ALPHA}  (OU relaxation)"
echo "# SIGMA_MIN        = ${SIGMA_MIN}"
echo "# SIGMA_MAX        = ${SIGMA_MAX}"
echo "# N_STEPS          = ${N_STEPS} (logarithmic steps)"
echo "# DT               = ${DT}     (time step)"
echo "# BETA             = ${BETA}   (recovery rate)"
echo "# LAMBDA           = ${LAMBDA} (infection decay)"
echo "# REALIZATION      = ${REALIZ} (realizations per sigma)"
echo "# OUTPUT_BASE      = ${OUTPUT_BASE}"
echo "# =========================================="
echo ""

# Loop over sigma values using logarithmic spacing
for ((i=0; i < N_STEPS; i++)); do
    # Calculate SIGMA using logspace formula
    # SIGMA = SIGMA_MIN * (SIGMA_MAX/SIGMA_MIN)^(i/(N_STEPS-1))
    if [ $i -eq 0 ]; then
        SIGMA=$SIGMA_MIN
    elif [ $i -eq $((N_STEPS-1)) ]; then
        SIGMA=$SIGMA_MAX
    else
        # logspace calculation using bc
        RATIO=$(echo "scale=10; l($SIGMA_MAX/$SIGMA_MIN)" | bc -l)
        EXPONENT=$(echo "scale=10; $i / ($N_STEPS - 1)" | bc -l)
        SIGMA=$(echo "scale=10; $SIGMA_MIN * e($RATIO * $EXPONENT)" | bc -l)
    fi
    
    # Format sigma with consistent decimal places
    SIGMA_FORMATTED=$(printf "%.4f" $SIGMA)
    
    # Create filename with all parameters
    FILENAME="${OUTPUT_BASE}/data_phi${PHI}_rc${RC}_N${N}_alpha${ALPHA}_sigma${SIGMA_FORMATTED}_beta${BETA}_lambda${LAMBDA}.dat"
    
    echo "# =========================================="
    echo "# Iteration $((i+1))/${N_STEPS}: SIGMA = ${SIGMA_FORMATTED}"
    echo "# Output file: ${FILENAME}"
    echo "# =========================================="
    
    # Compile with current sigma value
    CFLAGS="-Iinclude -DPHI=${PHI} -DRC=${RC} -DN=${N} -DALPHA=${ALPHA} -DSIGMA=${SIGMA} -DDT=${DT} -DBETA=${BETA} -DLAMBDA=${LAMBDA} -DREALIZATION=${REALIZ}"
    
    echo "# Compiling..."
    $GCC $CFLAGS $SRC $LDFLAGS -o $OUT 2>&1 | grep -i "error" && {
        echo "# ❌ Compilation failed for SIGMA=${SIGMA_FORMATTED}!"
        exit 1
    }
    
    # Run simulation and save results
    echo "# Running simulation..."
    ./$OUT > "$FILENAME"
    
    # Show file size
    FILE_SIZE=$(du -h "$FILENAME" | cut -f1)
    echo "# ✓ Saved to: ${FILENAME} (${FILE_SIZE})"
    echo ""
done

echo "# =========================================="
echo "# ✓ All simulations completed!"
echo "# Results saved in: ${OUTPUT_BASE}/"
echo "# =========================================="

# List generated directories and files
echo ""
echo "# Generated files:"
find "$OUTPUT_BASE" -type f -name "*.dat" | wc -l | awk '{print "# Total files: " $1}'
echo ""
ls -lh "$OUTPUT_BASE"/*.dat | tail -5 | awk '{print "# " $9 " (" $5 ")"}'