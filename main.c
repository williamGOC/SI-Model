#include "config.h"
#include "system.h"


int main(int argc, char const *argv[]) {

    srand(time(NULL));

    double rc = RC;
    double dt = 0.02;
    double d = 2;
    double z = 9;

    int nCells = L_BOX;

    systemSI *pS = makeSystem(rc, dt, d, z);

    verifyParticlesInCells(pS);

    //destroySystem(pS, nCells);

    return 0;
}
