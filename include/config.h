#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

// Box size calculated from density and number of particles
#define L_BOX (sqrt(N / PHI))

// Number of cells per dimension for spatial partitioning
#define N_BOX ((int)(L_BOX / RC))

// Maximum number of particles allowed per cell
#ifndef MAX_PARTICLES_PER_CELL
#define MAX_PARTICLES_PER_CELL 62
#endif

#endif // __CONFIG_H__