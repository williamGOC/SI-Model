#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdlib.h>
#include <math.h>
#include <time.h>

// =======================================================
//   Random number utilities for C simulations
// =======================================================

// Initialize random number generator (seed=0 uses time(NULL))
void seed_random(unsigned int);

// Return uniform random number in [0, 1)
double uniform_pos(void);

// Return uniform random number in [a, b)
double uniform_range(double, double);

// Return Gaussian random number with mean=0 and variance=1
double gasdev(void);

// Return Gaussian random number with mean=mu and std=sigma
double gasdev_mu_sigma(double, double);

// Alternative uniform generator based on Park-Miller algorithm
float rand2(long *);

// Initialize Gaussian random number generator
void seed_gasdev(unsigned int);

#endif