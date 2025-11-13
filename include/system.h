#ifndef __SYSTEM_H__
#define __SYSTEM_H__

// Cell structure for spatial partitioning
typedef struct {
    int nParticles;        // Number of particles in this cell
    int *particleIndex;    // Array of particle indices in this cell
} cell;

// Main system structure for SIS epidemic simulation
typedef struct {
    // Memory sizes for dynamic arrays
    size_t memoryX;            // Size of position arrays
    size_t memoryIndex;        // Size of index array
    size_t memoryState;        // Size of state arrays
    size_t memoryNeighborCell; // Size of neighbor cell array
    size_t memoryCellList;     // Size of cell list array
    
    // Particle data
    double *x;          // Current positions [x1, y1, x2, y2, ...]
    double *x0;         // Equilibrium positions (OU process centers)
    int *index;         // Particle indices
    int *state;         // Current epidemic state (0=Infected, 1=Susceptible)
    int *fakeState;     // Temporary state buffer for updates
    
    // Spatial partitioning structures
    cell *cellList;     // Array of cells for spatial hashing
    int *neighborCell;  // Neighbor cell indices for each cell
    
    // System parameters
    double dt;          // Time step
    double rc;          // Cutoff radius for interactions
    double alpha;       // OU process relaxation rate
    double sigma;       // OU process noise strength
    double cellSize;    // Size of each spatial cell
    int nCells;         // Number of cells per dimension
    int d;              // Spatial dimension (typically 2)
    int z;              // Number of neighbor cells (including self)
} systemSI;

// System initialization and cleanup
systemSI *makeSystem(double, double, double, double, int, int);
void destroySystem(systemSI *);

// Initial setup functions
void putParticles(systemSI *);       // Initialize particle positions randomly
void initialState(systemSI *);       // Set initial epidemic states

// Spatial partitioning functions
void getCellIndex(systemSI *);       // Assign particles to cells
void getNeighborList(systemSI *);    // Build neighbor cell list

// Dynamics functions
void iteration(systemSI *);          // Update particle positions (OU process)
void propagation_v00(systemSI *, double, double);  // Update epidemic states (version 0)
void propagation_v01(systemSI *, double, double);  // Update epidemic states (version 1)
void propagation_v02(systemSI *, double, double);  // Update epidemic states (version 2)

// Utility functions
void verifyParticlesInCells(systemSI *);  // Debug: verify cell assignment
double minImage(double, double);          // Compute minimum image distance (PBC)

#endif // __SYSTEM_H__