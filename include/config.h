#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =======================================================
// System parameters
// =======================================================

// System density (particles per unit area)
#ifndef PHI
#define PHI 0.9
#endif

// Cutoff radius for particle interactions
#ifndef RC
#define RC 2.5
#endif

// Number of particles in the system
#ifndef N
#define N 1000
#endif

// Ornstein-Uhlenbeck process relaxation rate
#ifndef ALPHA
#define ALPHA 5.0
#endif

// Ornstein-Uhlenbeck process noise strength
#ifndef SIGMA
#define SIGMA 0.5
#endif

// Number of realizations
#ifndef REALIZATION
#define REALIZATION 1000
#endif

// Time step for integration
#ifndef DT
#define DT 0.01
#endif

// =======================================================
// Epidemic parameters
// =======================================================

// Recovery rate (Infected -> Susceptible)
#ifndef BETA
#define BETA 0.5
#endif

// Spatial decay parameter for infection probability
#ifndef LAMBDA
#define LAMBDA 1.0
#endif

// =======================================================
// Geometry parameters
// =======================================================

// Spatial dimension (2D system)
#define DIM 2

// Number of neighbor cells (3x3 grid = 9 cells)
#define COORDINATION 9

// =======================================================
// Derived parameters
// =======================================================

// Box size calculated from density and number of particles
#define L_BOX (sqrt(N / PHI))

// Number of cells per dimension for spatial partitioning
#define N_BOX ((int)(L_BOX / RC))

// Maximum number of particles allowed per cell
#ifndef MAX_PARTICLES_PER_CELL
#define MAX_PARTICLES_PER_CELL 62
#endif

#endif // __CONFIG_H__