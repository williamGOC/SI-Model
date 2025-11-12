#include "config.h"
#include "system.h"

systemSI *makeSystem(double rc, double dt, double alpha, int d, int z) {

    // Memory allocation for a structure 'systemSI'
    systemSI *pS = (systemSI *)malloc(sizeof(systemSI));
    assert(pS != NULL);

    pS -> rc = rc;
    pS -> alpha = alpha;
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


void destroySystem(systemSI *pS, int nCells) {

    if (pS != NULL) {
        free(pS -> x);
        free(pS -> index);
        free(pS -> state);
        free(pS -> neighborCell);

        for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
            free(pS -> cellList[cellIdx].particleIndex);
        }

        free(pS -> cellList);
    }

    free(pS);
}



void putParticles(systemSI *pS) {

    int d = pS -> d;

    for(int i = 0; i < N; i++) {

        pS -> x[d * i + 0] = ((double)rand() / RAND_MAX) * L_BOX;
        pS -> x[d * i + 1] = ((double)rand() / RAND_MAX) * L_BOX;
    }
}


void initialState(systemSI *pS) {

    for (int i = 0; i < N; i++) {
        pS -> state[i] = 1;
    }

    // ...
    int j = (int)((rand() / RAND_MAX) * N);
    pS -> state[j] = 0;
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



void iteration(systemSI *pS){

    int d = pS -> d;
    double *x  = pS -> x;
    double *x0 = pS -> x0;
    double dt = pS -> dt;
    double alpha = pS -> alpha;

    double expAlphaDt = expf(-alpha * dt);

    for (int idx = 0; idx < N; idx++) {
        
        x[d * idx + 0] = x0[d * idx + 0] + (x[d * idx + 0] - x0[d * idx + 0]) * expAlphaDt + eta;
        x[d * idx + 1] = x0[d * idx + 1] + (x[d * idx + 1] - x0[d * idx + 1]) * expAlphaDt + eta;
    }
    
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