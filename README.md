# SIS Epidemic Model with Ornstein-Uhlenbeck Dynamics

A particle-based simulation of an SIS (Susceptible-Infected-Susceptible) epidemic model where particles move according to an Ornstein-Uhlenbeck process in a 2D periodic domain.

## Overview

This simulation combines:
- **Spatial dynamics**: Particles perform Ornstein-Uhlenbeck (OU) motion, oscillating around equilibrium positions with periodic boundary conditions
- **Epidemic dynamics**: Distance-dependent infection spreading following an SIS model with spatial interactions

## Model Description

### Particle Dynamics
Particles move via an Ornstein-Uhlenbeck process:
- Each particle has an equilibrium position `x₀`
- Position evolves as: `dx = -α(x - x₀)dt + σdW`
- Periodic boundary conditions (Pac-Man style wrapping)

### Epidemic Dynamics
- **States**: Susceptible (S, blue) or Infected (I, red)
- **Recovery**: Infected → Susceptible with rate `β`
- **Infection**: Probability from neighbor at distance `r`: `p(r) = exp(-λr) dt`
- **Total infection probability**: `P(infection) = 1 - ∏(1 - pⱼ)` over all infected neighbors

## Project Structure
```
.
├── include/
│   ├── config.h          # System parameters and constants
│   ├── system.h          # System structure and function declarations
│   └── random.h          # Random number generation utilities
├── src/
│   ├── system.c          # Core simulation functions
│   └── random.c          # Random number generators
├── move.c                # OpenGL visualization main
├── main.c                # Simple command-line main
├── run_move.sh           # Compilation script (with OpenGL)
└── run_main.sh           # Compilation script (no OpenGL)
```

## Compilation

### With OpenGL Visualization
```bash
./run_move.sh [PHI] [RC] [N] [ALPHA] [SIGMA] [DT] [BETA] [LAMBDA]
```

### Command-Line Only
```bash
./run_main.sh [PHI] [RC] [N] [ALPHA] [SIGMA] [DT] [BETA] [LAMBDA]
```

### Parameters
- `PHI`: Particle density (default: 0.9)
- `RC`: Cutoff radius for interactions (default: 2.5)
- `N`: Number of particles (default: 1000)
- `ALPHA`: OU relaxation rate (default: 5.0)
- `SIGMA`: OU noise strength (default: 0.5)
- `DT`: Time step (default: 0.01)
- `BETA`: Recovery rate I→S (default: 0.5)
- `LAMBDA`: Infection spatial decay (default: 1.0)

### Examples
```bash
# Use all defaults
./run_move.sh

# Custom parameters
./run_move.sh 0.9 2.5 1000 5.0 0.5 0.01 0.8 1.5
```

## Running the Simulation

### Visualization Mode
```bash
./move
```

**Controls:**
- `ESC`: Exit
- `p`: Pause/resume
- `g`: Toggle cell grid
- `+/-`: Zoom in/out
- `r`: Reset zoom
- `f/F`: Increase/decrease FPS
- `Arrow keys`: Pan view

### Data Collection Mode
```bash
./main > output.txt
./main | grep -v "^#" > data.txt
```

Output format:
```
step  time    S    I
0     0.0000  999  1
100   1.0000  985  15
200   2.0000  970  30
...
```

## Implementation Details

### Spatial Partitioning
- Uses cell lists for efficient neighbor searches
- Cell size = cutoff radius `RC`
- Each particle searches only in neighboring cells (3×3 grid)

### Propagation Models
Three versions available in `system.c`:
- `propagation_v00`: Independent transitions (no spatial interaction)
- `propagation_v01`: Count-based infection (linear in neighbor count)
- `propagation_v02`: Distance-dependent infection (exponential decay)

### Periodic Boundaries
- Minimum image convention for distance calculation
- Position wrapping using modulo arithmetic
- Ensures particles near boundaries interact correctly

## Key Functions

**System Management:**
- `makeSystem()`: Initialize simulation
- `destroySystem()`: Free memory
- `iteration()`: Update particle positions
- `propagation_v02()`: Update epidemic states

**Spatial Partitioning:**
- `getCellIndex()`: Assign particles to cells
- `getNeighborList()`: Build neighbor cell lists
- `minImage()`: Compute minimum distance (PBC)

## Dependencies

- **Standard libraries**: `stdio.h`, `stdlib.h`, `math.h`, `time.h`
- **OpenGL** (visualization only): `GL/gl.h`, `GL/glu.h`, `GL/glut.h`

## License

[Add your license here]

## Authors

- William G. C. Oropesa