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
    double dt = 0.01;       // Time step
    double alpha = 5.0;     // OU parameter
    double sigma = 0.5;     // OU noise
    int d = 2;              // Dimension
    int z = 9;              // Number of neighbor cells
    
    // Epidemiological parameters
    double beta = 0.5;      // Recovery rate (I -> S)
    double lambda = 1.0;    // Spatial decay of infection
    
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
    int printEvery = 10;
    
    printf("# Starting simulation...\n");
    printf("# Step\tTime\t\tS\tI\n");
    
    // Main simulation loop
    for (int step = 0; step <= nSteps; step++) {
        if (step % printEvery == 0) {
            countStates(pS, &nS, &nI);
            printf("%d\t%.4f\t\t%d\t%d\n", step, step * dt, nS, nI);
        }
        
        // Update system
        iteration(pS);           // Update particle positions
        getCellIndex(pS);        // Update cell lists
        propagation_v02(pS, beta, lambda);  // Update epidemic states
    }
    
    printf("# Simulation completed.\n");
    
    // Free memory
    destroySystem(pS);
    
    return 0;
}