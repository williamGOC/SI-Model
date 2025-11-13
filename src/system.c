#include "config.h"
#include "random.h"
#include "system.h"

// Create and initialize the system with given parameters
systemSI *makeSystem(double rc, double dt, double alpha, double sigma, int d, int z) {

    // Allocate memory for system structure
    systemSI *pS = (systemSI *)malloc(sizeof(systemSI));
    assert(pS != NULL);

    // Set system parameters
    pS->rc = rc;
    pS->alpha = alpha;
    pS->sigma = sigma;
    pS->dt = dt;
    pS->nCells = N_BOX;
    pS->cellSize = L_BOX / N_BOX;
    pS->d = d;
    pS->z = z;

    // Calculate memory sizes for arrays
    pS->memoryX = d * N * sizeof(double);
    pS->memoryIndex = N * sizeof(int);
    pS->memoryState = N * sizeof(int);
    pS->memoryNeighborCell = z * N_BOX * N_BOX * sizeof(int);
    pS->memoryCellList = N_BOX * N_BOX * sizeof(cell);

    // Allocate position arrays
    pS->x  = (double *)malloc(pS->memoryX);
    pS->x0 = (double *)malloc(pS->memoryX);
    assert(pS->x != NULL && pS->x0 != NULL);

    // Initialize particle positions randomly
    putParticles(pS);
    memcpy(pS->x0, pS->x, pS->memoryX);  // Copy to equilibrium positions

    // Allocate index array
    pS->index = (int *)malloc(pS->memoryIndex);
    assert(pS->index != NULL);

    // Allocate state arrays
    pS->state     = (int *)malloc(pS->memoryState);
    pS->fakeState = (int *)malloc(pS->memoryState);
    assert(pS->state != NULL && pS->fakeState != NULL);

    // Set initial epidemic states
    initialState(pS);

    // Allocate and build neighbor cell list
    pS->neighborCell = (int *)malloc(pS->memoryNeighborCell);
    assert(pS->neighborCell != NULL);
    getNeighborList(pS);

    // Allocate cell list structure
    pS->cellList = (cell *)malloc(pS->memoryCellList);
    assert(pS->cellList != NULL);

    // Initialize each cell
    for (int cellIdx = 0; cellIdx < N_BOX * N_BOX; cellIdx++) {
        pS->cellList[cellIdx].nParticles = 0;
        pS->cellList[cellIdx].particleIndex = (int *)calloc(MAX_PARTICLES_PER_CELL, sizeof(int));
    }
    
    // Assign particles to cells
    getCellIndex(pS);

    return pS;
}


// Free all memory allocated for the system
void destroySystem(systemSI *pS) {
    if (pS == NULL)
        return;

    // Free main arrays
    free(pS->x);
    free(pS->x0);
    free(pS->index);
    free(pS->state);
    free(pS->fakeState);
    free(pS->neighborCell);

    // Free cell list arrays
    if (pS->cellList != NULL) {
        for (int i = 0; i < pS->nCells * pS->nCells; i++) {
            free(pS->cellList[i].particleIndex);
            pS->cellList[i].particleIndex = NULL;
        }
        free(pS->cellList);
        pS->cellList = NULL;
    }

    // Free system structure
    free(pS);
}


// Initialize particle positions randomly in the box
void putParticles(systemSI *pS) {
    int d = pS->d;
    for (int i = 0; i < N; i++) {
        pS->x[d * i + 0] = ((double)rand() / (double)RAND_MAX) * L_BOX;
        pS->x[d * i + 1] = ((double)rand() / (double)RAND_MAX) * L_BOX;
    }
}


// Set initial epidemic state: all susceptible except one random infected
void initialState(systemSI *pS) {
    // All particles start as susceptible (state=1)
    for (int i = 0; i < N; i++) {
        pS->state[i] = 1;
    }

    // Choose one random particle to be infected (state=0)
    int j = (int)(((double)rand() / (double)RAND_MAX) * N);
    if (j < 0) j = 0;
    if (j >= N) j = N-1;
    pS->state[j] = 0;
}


// Assign particles to spatial cells based on their positions
void getCellIndex(systemSI *pS) {

    double *x = pS->x;
    double cellSize = pS->cellSize;
    int nCells = pS->nCells;
    int d = pS->d;

    // Clear all cells
    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        pS->cellList[cellIdx].nParticles = 0;
    }

    // Assign each particle to its cell
    for (int idx = 0; idx < N; idx++) {

        int ix = ((int)(x[d * idx + 0] / cellSize)) % nCells;
        int iy = ((int)(x[d * idx + 1] / cellSize)) % nCells;

        // Handle periodic boundary conditions
        if (ix < 0) ix += nCells;
        if (iy < 0) iy += nCells;

        int cellIdx = iy * nCells + ix;

        // Add particle to cell
        int current_count = pS->cellList[cellIdx].nParticles;
        pS->cellList[cellIdx].particleIndex[current_count] = idx;

        // Increment particle count
        pS->cellList[cellIdx].nParticles++;
    }
}


// Build list of neighbor cells for each cell (including self)
void getNeighborList(systemSI *pS) {

    double cellSize = pS->cellSize;
    int nCells = pS->nCells;
    int d = pS->d;
    int z = pS->z;

    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        
        int i = cellIdx % nCells;
        int j = cellIdx / nCells;

        int n = 0;
        // Loop over 3x3 grid of neighboring cells
        for (int dj = -1; dj <= 1; dj++) {
            for (int di = -1; di <= 1; di++) {

                // Apply periodic boundary conditions
                int ni = (i + di + nCells) % nCells;
                int nj = (j + dj + nCells) % nCells;

                pS->neighborCell[z * cellIdx + n] = ni + nj * nCells;
                n++;
            }
        }
    }
}


// Update particle positions using Ornstein-Uhlenbeck process with periodic boundaries
void iteration(systemSI *pS) {
    int d = pS->d;
    double *x  = pS->x;
    double *x0 = pS->x0;
    double dt = pS->dt;
    double alpha = pS->alpha;
    double sigma = pS->sigma;
    double L = L_BOX;

    // Precompute OU process parameters
    double exp_md = exp(-alpha * dt);
    double tmp = 1.0 - exp(-2.0 * alpha * dt);
    double var_factor = sigma * sqrt(tmp / (2.0 * alpha));

    // Update each particle position
    for (int idx = 0; idx < N; idx++) {
        int base_idx = d * idx;
        
        for (int mu = 0; mu < d; mu++) {
            int pos = base_idx + mu;
            
            double cur = x[pos];
            double eq  = x0[pos];
            double z   = gasdev_mu_sigma(0.0, 1.0);

            // Use minimum image convention for periodic boundaries
            double diff = minImage(cur, eq);
            double newx = eq + diff * exp_md + var_factor * z;

            // Apply periodic boundary conditions (wrap around)
            newx = fmod(newx, L);
            if (newx < 0.0) newx += L;

            x[pos] = newx;
        }
    }
}


// Version 0: Independent state transitions (no spatial interactions)
void propagation_v00(systemSI *pS, double beta, double lambda) {
    // Copy current state to buffer
    memcpy(pS->fakeState, pS->state, pS->memoryState);

    int *state     = pS->state;
    int *fakeState = pS->fakeState;
    double dt = pS->dt;

    for (int idx = 0; idx < N; idx++) {
        double r = uniform_pos();

        if (state[idx] == 0) {
            // Infected -> Susceptible with rate lambda
            fakeState[idx] = (r < lambda * dt) ? 1 : 0;
        } else {
            // Susceptible -> Infected with rate beta
            fakeState[idx] = (r < beta * dt) ? 0 : 1;
        }
    }

    // Update system state
    memcpy(state, fakeState, pS->memoryState);
}


// Version 1: Infection rate proportional to number of infected neighbors
void propagation_v01(systemSI *pS, double beta, double lambda) {

    memcpy(pS->fakeState, pS->state, pS->memoryState);
    
    int *state     = pS->state;
    int *fakeState = pS->fakeState;
    
    double dt = pS->dt;
    double rc = pS->rc;
    double L  = L_BOX;
    
    int d = pS->d;
    int z = pS->z;
    
    double *x  = pS->x;
    int nCells = pS->nCells;
    
    // Update cell lists
    getCellIndex(pS);
    
    for (int idx = 0; idx < N; idx++) {
        double r = uniform_pos();
        
        if (state[idx] == 0) {
            // Infected -> Susceptible with rate lambda
            fakeState[idx] = (r < lambda * dt) ? 1 : 0;
        } else {
            // Susceptible: count infected neighbors
            int num_infected_neighbors = 0;
            
            double xi = x[d * idx + 0];
            double yi = x[d * idx + 1];
            
            // Find particle's cell
            int ix = ((int)(xi / pS->cellSize)) % nCells;
            int iy = ((int)(yi / pS->cellSize)) % nCells;
            
            if (ix < 0) ix += nCells;
            if (iy < 0) iy += nCells;
            
            int cellIdx = iy * nCells + ix;
            
            // Search in neighboring cells only
            for (int n = 0; n < z; n++) {
                int neighborCellIdx = pS->neighborCell[z * cellIdx + n];
                
                for (int p = 0; p < pS->cellList[neighborCellIdx].nParticles; p++) {
                    int jdx = pS->cellList[neighborCellIdx].particleIndex[p];
                    
                    if (jdx == idx) continue;
                    if (state[jdx] != 0) continue; // Only count infected
                    
                    double xj = x[d * jdx + 0];
                    double yj = x[d * jdx + 1];
                    
                    double dx = minImage(xi, xj);
                    double dy = minImage(yi, yj);
                    double dist_sq = dx*dx + dy*dy;
                    
                    if (dist_sq < rc*rc) {
                        num_infected_neighbors++;
                    }
                }
            }
            
            // Infection probability proportional to infected neighbors
            double infection_prob = 1.0 - exp(-beta * num_infected_neighbors * dt);
            fakeState[idx] = (r < infection_prob) ? 0 : 1;
        }
    }
    
    memcpy(state, fakeState, pS->memoryState);
}


// Version 2: Distance-dependent infection probability exp(-lambda*r)
void propagation_v02(systemSI *pS, double beta, double lambda) {
    memcpy(pS->fakeState, pS->state, pS->memoryState);
    
    int *state     = pS->state;
    int *fakeState = pS->fakeState;
    double dt = pS->dt;
    double rc = pS->rc;
    int d = pS->d;
    int z = pS->z;
    double *x = pS->x;
    int nCells = pS->nCells;
    
    // Update cell lists
    getCellIndex(pS);
    
    for (int idx = 0; idx < N; idx++) {
        double r_random = uniform_pos();
        
        if (state[idx] == 0) {
            // Infected -> Susceptible with rate beta (recovery)
            fakeState[idx] = (r_random < beta * dt) ? 1 : 0;
        } else {
            // Susceptible: calculate probability of NOT being infected (product)
            double prob_no_infection = 1.0;
            
            double xi = x[d * idx + 0];
            double yi = x[d * idx + 1];
            
            // Find particle's cell
            int ix = ((int)(xi / pS->cellSize)) % nCells;
            int iy = ((int)(yi / pS->cellSize)) % nCells;
            if (ix < 0) ix += nCells;
            if (iy < 0) iy += nCells;
            int cellIdx = iy * nCells + ix;
            
            // Search for infected neighbors
            for (int n = 0; n < z; n++) {
                int neighborCellIdx = pS->neighborCell[z * cellIdx + n];
                
                for (int p = 0; p < pS->cellList[neighborCellIdx].nParticles; p++) {
                    int jdx = pS->cellList[neighborCellIdx].particleIndex[p];
                    
                    if (jdx == idx) continue;
                    if (state[jdx] != 0) continue; // Only infected
                    
                    double xj = x[d * jdx + 0];
                    double yj = x[d * jdx + 1];
                    
                    double dx = minImage(xi, xj);
                    double dy = minImage(yi, yj);
                    double dist = sqrt(dx*dx + dy*dy);
                    
                    if (dist < rc) {
                        // P(this neighbor infects me) = exp(-lambda*r) * dt
                        double p_infection_from_j = exp(-lambda * dist) * dt;
                        
                        // P(this neighbor does NOT infect me)
                        double p_no_infection_from_j = 1.0 - p_infection_from_j;
                        
                        // Multiply in product
                        prob_no_infection *= p_no_infection_from_j;
                    }
                }
            }
            
            // P(infection) = 1 - P(no infection)
            double infection_prob = 1.0 - prob_no_infection;
            
            fakeState[idx] = (r_random < infection_prob) ? 0 : 1;
        }
    }
    
    memcpy(state, fakeState, pS->memoryState);
}


// Compute minimum image distance for periodic boundary conditions
double minImage(double xi, double xj){
    double xij = xi - xj;
    return xij - L_BOX * round(xij / L_BOX); 
}


// Debug function: verify particle assignment to cells
void verifyParticlesInCells(systemSI *pS) {
    printf("\n=== VERIFICATION: PARTICLES PER CELL ===\n\n");
    
    int nCells = pS->nCells;
    int total = 0;
    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        int nParticles = pS->cellList[cellIdx].nParticles;
        printf("cell %d: %d particles\n", cellIdx, nParticles);
        total += nParticles;
    }

    printf("Total: %d\n", total);
}