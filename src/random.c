#include "random.h"

static int seeded = 0;

// =======================================================
//   Inicialización
// =======================================================
void seed_random(unsigned int seed) {
    if (seed == 0)
        seed = (unsigned int)time(NULL);
    srand(seed);
    seeded = 1;
}

// =======================================================
//   Uniforme [0,1)
// =======================================================
double uniform_pos(void) {
    if (!seeded) seed_random(0);
    return rand() / (RAND_MAX + 1.0);
}

// Uniforme [a,b)
double uniform_range(double a, double b) {
    return a + (b - a) * uniform_pos();
}

// =======================================================
//   Distribución Gaussiana (Box–Muller)
// =======================================================
double gasdev(void) {
    static int haveSpare = 0;
    static double spare;

    if (haveSpare) {
        haveSpare = 0;
        return spare;
    }

    double u, v, s;
    do {
        u = 2.0 * uniform_pos() - 1.0;
        v = 2.0 * uniform_pos() - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    s = sqrt(-2.0 * log(s) / s);
    spare = v * s;
    haveSpare = 1;
    return u * s;
}

// Gaussiana con media y desviación arbitrarias
double gasdev_mu_sigma(double mu, double sigma) {
    return mu + sigma * gasdev();
}

// =======================================================
//   rand2() - versión Park-Miller (Numerical Recipes)
// =======================================================
float rand2(long *idum) {
    const long IA = 16807;
    const long IM = 2147483647;
    const double AM = (1.0 / IM);
    const long IQ = 127773;
    const long IR = 2836;
    const long MASK = 123459876;
    long k;
    float ans;

    *idum ^= MASK;
    k = (*idum) / IQ;
    *idum = IA * (*idum - k * IQ) - IR * k;
    if (*idum < 0) *idum += IM;
    ans = AM * (*idum);
    *idum ^= MASK;
    return ans;
}


// =======================================================
// Función para compatibilidad con elasticity.c
void seed_gasdev(unsigned int seed) {
    seed_random(seed);
}