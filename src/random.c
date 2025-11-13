#include "random.h"

static int seeded = 0;

// =======================================================
//   Initialization
// =======================================================

// Initialize random number generator with given seed (0 uses current time)
void seed_random(unsigned int seed) {
    if (seed == 0)
        seed = (unsigned int)time(NULL);
    srand(seed);
    seeded = 1;
}

// =======================================================
//   Uniform distribution [0,1)
// =======================================================

// Return uniform random number in [0,1)
double uniform_pos(void) {
    if (!seeded) seed_random(0);
    return rand() / (RAND_MAX + 1.0);
}

// Return uniform random number in [a,b)
double uniform_range(double a, double b) {
    return a + (b - a) * uniform_pos();
}

// =======================================================
//   Gaussian distribution (Box-Muller algorithm)
// =======================================================

// Return Gaussian random number with mean=0 and std=1
double gasdev(void) {
    static int haveSpare = 0;
    static double spare;
    
    // Use spare value from previous call if available
    if (haveSpare) {
        haveSpare = 0;
        return spare;
    }
    
    // Box-Muller transform
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

// Return Gaussian random number with mean=mu and std=sigma
double gasdev_mu_sigma(double mu, double sigma) {
    return mu + sigma * gasdev();
}

// =======================================================
//   Park-Miller generator (Numerical Recipes)
// =======================================================

// Alternative uniform generator using Park-Miller algorithm
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
//   Compatibility function
// =======================================================

// Initialize Gaussian generator (wrapper for seed_random)
void seed_gasdev(unsigned int seed) {
    seed_random(seed);
}