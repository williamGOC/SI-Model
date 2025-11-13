#include "config.h"
#include "random.h"
#include "system.h"

systemSI *makeSystem(double rc, double dt, double alpha, double sigma, int d, int z) {

    // Memory allocation for a structure 'systemSI'
    systemSI *pS = (systemSI *)malloc(sizeof(systemSI));
    assert(pS != NULL);

    pS -> rc = rc;
    pS -> alpha = alpha;
    pS -> sigma = sigma;
    pS -> dt = dt;
    pS -> nCells = N_BOX;
    pS -> cellSize = L_BOX / N_BOX;
    pS -> d = d;
    pS -> z = z;

    pS -> memoryX = d * N * sizeof(double);
    pS -> memoryIndex = N * sizeof(int);
    pS -> memoryState = N * sizeof(int);
    pS -> memoryNeighborCell = z * N_BOX * N_BOX * sizeof(int);
    pS -> memoryCellList = N_BOX * N_BOX * sizeof(cell);

    pS -> x  = (double *)malloc(pS -> memoryX);
    pS -> x0 = (double *)malloc(pS -> memoryX);
    assert(pS -> x != NULL && pS -> x0 != NULL);

    putParticles(pS);
    memcpy(pS -> x0, pS -> x, pS -> memoryX);

    pS -> index = (int *)malloc(pS -> memoryIndex);
    assert(pS -> index != NULL);

    pS -> state     = (int *)malloc(pS -> memoryState);
    pS -> fakeState = (int *)malloc(pS -> memoryState);
    assert(pS -> state != NULL && pS -> fakeState != NULL);

    initialState(pS);

    pS -> neighborCell = (int *)malloc(pS -> memoryNeighborCell);
    assert(pS -> neighborCell != NULL);

    getNeighborList(pS);

    pS -> cellList = (cell *)malloc(pS -> memoryCellList);
    assert(pS -> cellList != NULL);

    for (int cellIdx = 0; cellIdx < N_BOX * N_BOX; cellIdx++) {
        
        pS -> cellList[cellIdx].nParticles = 0;
        pS -> cellList[cellIdx].particleIndex = (int *)calloc(MAX_PARTICLES_PER_CELL, sizeof(int));
    }
    
    getCellIndex(pS);

    return pS;

}


void destroySystem(systemSI *pS) {
    if (pS == NULL)
        return;

    // Liberar vectores principales
    free(pS->x);
    free(pS->x0);
    free(pS->index);
    free(pS->state);
    free(pS -> fakeState);
    free(pS->neighborCell);

    // Liberar listas de celdas
    if (pS->cellList != NULL) {
        for (int i = 0; i < pS->nCells * pS->nCells; i++) {
            free(pS->cellList[i].particleIndex);
            pS->cellList[i].particleIndex = NULL;
        }
        free(pS->cellList);
        pS->cellList = NULL;
    }

    // Finalmente, liberar el sistema
    free(pS);
}


void putParticles(systemSI *pS) {
    int d = pS->d;
    for (int i = 0; i < N; i++) {
        pS->x[d * i + 0] = ((double)rand() / (double)RAND_MAX) * L_BOX;
        pS->x[d * i + 1] = ((double)rand() / (double)RAND_MAX) * L_BOX;
    }
}



void initialState(systemSI *pS) {
    for (int i = 0; i < N; i++) {
        pS->state[i] = 1;
    }

    // Elegir aleatoriamente una partícula y ponerla en estado 0
    int j = (int)(((double)rand() / (double)RAND_MAX) * N);
    if (j < 0) j = 0;
    if (j >= N) j = N-1;
    pS->state[j] = 0;
}



void getCellIndex(systemSI *pS) {

    double *x = pS -> x;
    double cellSize = pS -> cellSize;
    int nCells = pS -> nCells;
    int d = pS -> d;

    // PASO 1: Limpiar las celdas (resetear contador)
    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        pS->cellList[cellIdx].nParticles = 0;
    }

    for (int idx = 0; idx < N; idx++) {

        int ix = ((int)(x[d * idx + 0] / cellSize)) % nCells;
        int iy = ((int)(x[d * idx + 1] / cellSize)) % nCells;

        // check
        if (ix < 0) ix += nCells;
        if (iy < 0) iy += nCells;

        int cellIdx = iy * nCells + ix;

        // AGREGAR la partícula a la celda
        int current_count = pS -> cellList[cellIdx].nParticles;
        pS -> cellList[cellIdx].particleIndex[current_count] = idx;

        // INCREMENTAR el contador
        pS -> cellList[cellIdx].nParticles++;

    }
}


void getNeighborList(systemSI *pS) {

    double cellSize = pS -> cellSize;
    int nCells = pS -> nCells;
    int d = pS -> d;
    int z = pS -> z;

    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        
        int i = cellIdx % nCells;
        int j = cellIdx / nCells;

        int n = 0;
        for (int dj = -1; dj <= 1; dj++) {
            for (int di = -1; di <= 1; di++) {

                int ni = (i + di + nCells) % nCells;
                int nj = (j + dj + nCells) % nCells;

                pS -> neighborCell[z * cellIdx + n] = ni + nj * nCells;
                n++;
            }
        }
    }
}


void iteration(systemSI *pS) {
    int d = pS->d;
    double *x  = pS->x;
    double *x0 = pS->x0;
    double dt = pS->dt;
    double alpha = pS->alpha;
    double sigma = pS->sigma;
    double L = L_BOX;

    // Parámetros del proceso de Ornstein-Uhlenbeck (precalculados una sola vez)
    double exp_md = exp(-alpha * dt);
    double tmp = 1.0 - exp(-2.0 * alpha * dt);
    double var_factor = sigma * sqrt(tmp / (2.0 * alpha));

    // Loop optimizado
    for (int idx = 0; idx < N; idx++) {
        int base_idx = d * idx;
        
        for (int mu = 0; mu < d; mu++) {
            int pos = base_idx + mu;
            
            double cur = x[pos];
            double eq  = x0[pos];
            double z   = gasdev_mu_sigma(0.0, 1.0);

            // Cálculo de nueva posición
            double diff = minImage(cur, eq);
            double newx = eq + diff * exp_md + var_factor * z;

            // Condiciones periódicas optimizadas (estilo Pac-Man)
            // Usamos fmod para mayor robustez
            newx = fmod(newx, L);
            if (newx < 0.0) newx += L;

            x[pos] = newx;
        }
    }
}



void propagation_v00(systemSI *pS, double beta, double lambda) {
    // Copia de estado actual
    memcpy(pS->fakeState, pS->state, pS->memoryState);

    int *state     = pS->state;
    int *fakeState = pS->fakeState;
    double dt = pS->dt;

    for (int idx = 0; idx < N; idx++) {
        double r = uniform_pos();  // número aleatorio uniforme en [0,1)

        if (state[idx] == 0) {
            // Infectado -> Susceptible con tasa λ
            fakeState[idx] = (r < lambda * dt) ? 1 : 0;
        } else {
            // Susceptible -> Infectado con tasa β
            fakeState[idx] = (r < beta * dt) ? 0 : 1;
        }
    }

    // Actualiza el estado del sistema
    memcpy(state, fakeState, pS->memoryState);
}


void propagation_v01(systemSI *pS, double beta, double lambda) {

    memcpy(pS -> fakeState, pS -> state, pS -> memoryState);
    
    int *state     = pS->state;
    int *fakeState = pS->fakeState;
    
    double dt = pS->dt;
    double rc = pS -> rc;
    double L  = L_BOX;
    
    int d = pS -> d;
    int z = pS -> z;
    
    double *x  = pS -> x;
    int nCells = pS -> nCells;
    
    // Actualizar lista de celdas
    getCellIndex(pS);
    
    for (int idx = 0; idx < N; idx++) {
        double r = uniform_pos();
        
        if (state[idx] == 0) {
            // Infectado -> Susceptible con tasa λ
            fakeState[idx] = (r < lambda * dt) ? 1 : 0;
        } else {
            // Susceptible: contar vecinos infectados
            int num_infected_neighbors = 0;
            
            double xi = x[d * idx + 0];
            double yi = x[d * idx + 1];
            
            // Encontrar celda de la partícula i
            int ix = ((int)(xi / pS -> cellSize)) % nCells;
            int iy = ((int)(yi / pS -> cellSize)) % nCells;
            
            if (ix < 0) ix += nCells;
            if (iy < 0) iy += nCells;
            
            int cellIdx = iy * nCells + ix;
            
            // Buscar solo en celdas vecinas
            for (int n = 0; n < z; n++) {
                int neighborCellIdx = pS->neighborCell[z * cellIdx + n];
                
                for (int p = 0; p < pS -> cellList[neighborCellIdx].nParticles; p++) {
                    int jdx = pS -> cellList[neighborCellIdx].particleIndex[p];
                    
                    if (jdx == idx) continue;
                    if (state[jdx] != 0) continue; // Solo infectados
                    
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
            
            // Infección proporcional a vecinos infectados
            double infection_prob = 1.0 - exp(-beta * num_infected_neighbors * dt);
            fakeState[idx] = (r < infection_prob) ? 0 : 1;
        }
    }
    
    memcpy(state, fakeState, pS->memoryState);
}


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
    
    // Actualizar lista de celdas
    getCellIndex(pS);
    
    for (int idx = 0; idx < N; idx++) {
        double r_random = uniform_pos();
        
        if (state[idx] == 0) {
            // Infectado -> Susceptible con tasa β (recuperación)
            fakeState[idx] = (r_random < beta * dt) ? 1 : 0;
        } else {
            // Susceptible: calcular probabilidad de NO infectarse (productoria)
            double prob_no_infection = 1.0;
            
            double xi = x[d * idx + 0];
            double yi = x[d * idx + 1];
            
            // Encontrar celda
            int ix = ((int)(xi / pS->cellSize)) % nCells;
            int iy = ((int)(yi / pS->cellSize)) % nCells;
            if (ix < 0) ix += nCells;
            if (iy < 0) iy += nCells;
            int cellIdx = iy * nCells + ix;
            
            // Buscar vecinos infectados
            for (int n = 0; n < z; n++) {
                int neighborCellIdx = pS->neighborCell[z * cellIdx + n];
                
                for (int p = 0; p < pS->cellList[neighborCellIdx].nParticles; p++) {
                    int jdx = pS->cellList[neighborCellIdx].particleIndex[p];
                    
                    if (jdx == idx) continue;
                    if (state[jdx] != 0) continue; // Solo infectados
                    
                    double xj = x[d * jdx + 0];
                    double yj = x[d * jdx + 1];
                    
                    double dx = minImage(xi, xj);
                    double dy = minImage(yi, yj);
                    double dist = sqrt(dx*dx + dy*dy);
                    
                    if (dist < rc) {
                        // P(este vecino me infecta) = exp(-λ*r) * dt
                        double p_infection_from_j = exp(-lambda * dist) * dt;
                        
                        // P(este vecino NO me infecta)
                        double p_no_infection_from_j = 1.0 - p_infection_from_j;
                        
                        // Productoria
                        prob_no_infection *= p_no_infection_from_j;
                    }
                }
            }
            
            // P(infectarse) = 1 - P(no infectarse)
            double infection_prob = 1.0 - prob_no_infection;
            
            fakeState[idx] = (r_random < infection_prob) ? 0 : 1;
        }
    }
    
    memcpy(state, fakeState, pS->memoryState);
}

double minImage(double xi, double xj){
    double xij = xi - xj;
    return xij - L_BOX * round(xij / L_BOX); 
}


void verifyParticlesInCells(systemSI *pS) {
    printf("\n=== VERIFICACIÓN: PARTÍCULAS POR CELDA ===\n\n");
    
    int nCells = pS -> nCells;
    int total = 0;
    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        int nParticles = pS -> cellList[cellIdx].nParticles;
        printf("celll %d: %d particles\n", cellIdx, nParticles);
        total += nParticles;
    }

    printf("%d\n", total);


}