#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "random.h"
#include "system.h"

// Count the number of susceptible and infected particles
void countStates(systemSI *pS, int *nSusceptible, int *nInfected) {
    *nSusceptible = 0;
    *nInfected = 0;
    
    for (int i = 0; i < N; i++) {
        if (pS->state[i] == 0) {
            (*nInfected)++;
        } else {
            (*nSusceptible)++;
        }
    }
}

int main() {

    // Initialize random seed
    srand(time(NULL));
    seed_random(0);
    
    // System parameters
    double rc = RC;        // Cutoff radius
    double dt = DT;       // Time step
    double alpha = ALPHA;     // OU parameter
    double sigma = SIGMA;     // OU noise
    int d = DIM;              // Dimension
    int z = COORDINATION;              // Number of neighbor cells
    
    // Epidemiological parameters
    double beta = BETA;      // Recovery rate (I -> S)
    double lambda = LAMBDA;    // Spatial decay of infection
    
    // Create system
    printf("# Creating system...\n");
    systemSI *pS = makeSystem(rc, dt, alpha, sigma, d, z);
    printf("# System created with N=%d particles\n\n", N);
    
    // Initial state
    int nS, nI;
    countStates(pS, &nS, &nI);
    printf("# Initial state:\n");
    printf("# Susceptibles: %d, Infected: %d\n\n", nS, nI);
    
    // Simulation parameters
    int nSteps = 10000;
    
    printf("# Starting simulation...\n");
    printf("# Relz\tStep\tTime\tsigma\tR0\n");
    
    for (int relz = 0; relz < REALIZATION; relz++) {
        
        // Main simulation loop
        int idx0 = pS -> idx0;
        int r0 = 0;
        int step; for (step = 0; step <= nSteps && !pS -> state[idx0]; step++) {

            // Update system
            iteration(pS);                            // Update particle positions
            getCellIndex(pS);                         // Update cell lists
            r0  = propagation_v04(pS, beta, lambda);
        }

    
        //countStates(pS, &nS, &nI);
        printf("%d\t%d\t%.4f\t%d\n", relz, step, step * dt, r0);

        pS -> sigma = sigma;
        
        // Set initial epidemic states
        initialState(pS);

    }
    
    printf("# Simulation completed.\n");
    
    // Free memory
    destroySystem(pS);
    
    return 0;
}