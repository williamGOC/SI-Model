#ifndef __SYSTEM_H__
#define __SYSTEM_H__



typedef struct {

    int nParticles;
    int *particleIndex;

} cell;



typedef struct {
    
    // sizes of vectors
    size_t memoryX;
    size_t memoryIndex;
    size_t memoryState;
    size_t memoryNeighborCell;
    size_t memoryCellList;

    // state of each particle
    double *x;
    double *x0;
    int *index;
    int *state;
    int *fakeState;

    cell *cellList;

    // SystemSI vectors
    int *neighborCell;

    
    // SystemSI parameters
    double dt;
    double rc;
    double alpha;
    double sigma;
    double cellSize;
    int nCells;
    int d;
    int z;

} systemSI;


systemSI *makeSystem(double, double, double, double, int, int);
void destroySystem(systemSI *);

void putParticles(systemSI *);
void initialState(systemSI *);

void getCellIndex(systemSI *);
void getNeighborList(systemSI *);

void iteration(systemSI *);
void propagation(systemSI *, double, double);
void verifyParticlesInCells(systemSI *);

#endif // __SYSTEM_H__