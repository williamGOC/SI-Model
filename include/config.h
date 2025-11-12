#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef PHI
#define PHI 0.9
#endif

#ifndef RC
#define RC 2.5
#endif

#ifndef N
#define N 1000
#endif

#define L_BOX (sqrt(N / PHI))
#define N_BOX ((int)(L_BOX / RC))

#ifndef MAX_PARTICLES_PER_CELL
#define MAX_PARTICLES_PER_CELL 62
#endif

#endif // __CONFIG_H__