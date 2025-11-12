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

    pS -> state = (int *)malloc(pS -> memoryState);
    assert(pS -> state != NULL);

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
    double *x0 = pS->x0;   // punto de equilibrio por partícula (según tu definición)
    double dt = pS->dt;
    double alpha = pS->alpha;
    double sigma = pS->sigma;
    double L = L_BOX;

    // Precalcula exponencial (estable)
    double exp_md = exp(-alpha * dt);
    // varianza del término estocástico: sigma^2/(2 alpha) * (1 - exp(-2 alpha dt))
    double var_factor;
    if (alpha > 1e-14) {
        // uso de exp(-2 alpha dt) directo; se puede usar expm1 para más precisión si dt*alpha pequeño
        double tmp = 1.0 - exp(-2.0 * alpha * dt);
        var_factor = sigma * sqrt(tmp / (2.0 * alpha));
    } else {
        // límite alpha -> 0: var -> sigma^2 * dt  => std = sigma * sqrt(dt)
        var_factor = sigma * sqrt(dt);
        exp_md = 1.0; // cuando alpha ~ 0 no hay decaimiento
    }

    for (int idx = 0; idx < N; idx++) {
        for (int mu = 0; mu < d; mu++) {
            double cur = x[d * idx + mu];
            double eq  = x0[d * idx + mu];               // x0 puede ser distinto por partícula
            double z   = gasdev_mu_sigma(0.0, 1.0);      // N(0,1) estándar
            double newx = eq + (cur - eq) * exp_md + var_factor * z;

            // Condición periódica en [0, L)
            if (newx < 0.0) {
                // añadir múltiplos de L hasta entrar en rango (robusto si noise grande)
                newx = newx - floor(newx / L) * L;
            } else if (newx >= L) {
                newx = newx - floor(newx / L) * L;
            }

            x[d * idx + mu] = newx;
        }
    }
}


/*void iteration(systemSI *pS) {
    int d = pS->d;
    double *x  = pS->x;
    double dt = pS->dt;

    // TEST: Movimiento forzado simple
    for (int idx = 0; idx < N; idx++) {
        for (int mu = 0; mu < d; mu++) {
            x[d * idx + mu] += 0.5;  // Suma 0.5 cada frame
            
            // Condición periódica
            if (x[d * idx + mu] >= L_BOX) {
                x[d * idx + mu] -= L_BOX;
            }
        }
    }
}*/

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