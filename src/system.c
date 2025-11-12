#include "config.h"
#include "system.h"

system *makeSystem(double rc, double dt, int d, int z) {

    // Memory allocation for a structure 'system'
    system *pS = (system *)malloc(sizeof(system));
    assert(pS != NULL);

    int nCells = (int)(LX / rc);

    pS -> rc = rc;
    pS -> nCells = nCells;
    pS -> cellSize = LX / nCells;
    pS -> d = d;
    pS -> z = z;

    pS -> memoryX = d * N * sizeof(double);
    pS -> memoryIndex = N * sizeof(int);
    pS -> memoryState = N * sizeof(int);
    pS -> memoryNeighborCell = z * nCells * nCells * sizeof(int);
    pS -> memoryCellList = nCells * sizeof(cell);

    pS -> x = (double *)malloc(pS -> memoryX);
    assert(pS -> x != NULL);

    putParticles(pS);

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

    for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
        
        pS -> cellList[cellIdx].nParticles = 0;
        pS -> cellList[cellIdx].particleIndex = (int *)calloc(MAX_PARTICLES_PER_CELL, sizeof(int));
    }
    
    getCellIndex(pS);

    return pS;

}


void destroySystem(system *pS) {

    int nCells = pS -> nCells;

    if (pS != NULL) {
        free(pS -> x);
        free(pS -> index);
        free(pS -> state);
        free(pS -> neighborCell);

        for (int cellIdx = 0; cellIdx < nCells * nCells; cellIdx++) {
            free(pS -> cellList[cellIdx].particleIndex);
        }

        free(pS -> cellList);
        free(pS);
    }
}



void putParticles(system *pS) {

    int d = pS -> d;

    for(int i = 0; i < N; i++) {

        pS -> x[d * i + 0] = (rand() / RAND_MAX) * LX;
        pS -> x[d * i + 1] = (rand() / RAND_MAX) * LX;
    }
}


void initialState(system *pS) {

    for (int i = 0; i < N; i++) {
        pS -> state[i] = 1;
    }

    // ...
    int j = (int)((rand() / RAND_MAX) * N);
    pS -> state[j] = 0;
}


void getCellIndex(system *pS) {

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


void getNeighborList(system *pS) {

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