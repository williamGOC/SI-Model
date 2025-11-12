#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <stdlib.h>
#include <math.h>
#include <time.h>

// =======================================================
//   Random number utilities for C simulations
// =======================================================

// Inicializa el generador de números aleatorios
// Si seed == 0, usa time(NULL)
void seed_random(unsigned int);

// Devuelve un número uniforme en [0, 1)
double uniform_pos(void);

// Devuelve un número uniforme en [a, b)
double uniform_range(double, double);

// Devuelve un número gaussiano con media 0 y varianza 1
double gasdev(void);

// Devuelve un número gaussiano con media mu y desviación sigma
double gasdev_mu_sigma(double, double);

// Versión alternativa de uniform_pos basada en el generador de Park-Miller
float rand2(long *);

void seed_gasdev(unsigned int);

#endif
