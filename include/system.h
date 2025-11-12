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
    int *index;
    int *state;

    cell *cellList;

    // System vectors
    int *neighborCell;

    
    // System parameters
    double dt;
    double rc;
    double cellSize;
    int nCells;
    int d;
    int z;

} system;


system *makeSystem(double, double, int, int);
void destroySystem(system *);

void putParticles(system *);
void initialState(system *);

void getCellIndex(system *);


#endif // __SYSTEM_H__