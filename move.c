#include "config.h"
#include "random.h"
#include "system.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// Particle visual size for rendering
#define PARTICLE_RADIUS 0.2

// Number of segments to approximate circles
#define NUM_SEGMENTS 16

systemSI *pS = NULL;

// Zoom settings for orthographic projection
double zoomLeft;
double zoomRight;
double zoomBottom;
double zoomTop;

GLfloat zoomFactor = 1.1;

// Default window dimensions
GLint windowWidth  = 800;
GLint windowHeight = 800;

// Global visualization parameters
GLint FPS = 20;      // Frames per second
GLint viewMode = 0;  // Display mode toggle
GLint pause = 1;     // Pause/resume simulation
GLint nView = 1;     // Number of available views
GLint gridView = 0;  // Show/hide cell grid

void saveFrameToPPM(int frame) {
    unsigned char *pixels = (unsigned char *)malloc(3 * windowWidth * windowHeight);
    if (!pixels) {
        fprintf(stderr, "Error: no se pudo reservar memoria para pixels.\n");
        return;
    }

    // Leer los píxeles de la ventana actual
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Guardar en formato PPM (simple y sin compresión)
    char filename[64];
    sprintf(filename, "frames/frame_%05d.ppm", frame);
    FILE *f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: no se pudo abrir %s para escritura.\n", filename);
        free(pixels);
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", windowWidth, windowHeight);
    fwrite(pixels, 3, windowWidth * windowHeight, f);
    fclose(f);
    free(pixels);
}

// Set initial zoom to fit the entire simulation box
void setZoomForSystem() {
    zoomLeft   = 0.0;
    zoomRight  = L_BOX;
    zoomBottom = 0;
    zoomTop    = L_BOX;
}


// Initialize OpenGL and create the simulation system
void initOpenGL() {

    glClearColor(1.0, 1.0, 1.0, 1.0);  // White background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Initialize random number generators
    srand(time(NULL));
    seed_random(0);

    // System parameters
    double rc = RC;
    double dt = 0.01;
    double alpha = ALPHA;
    double sigma = SIGMA;
    double d = DIM;
    double z = COORDINATION;

    int nCells = L_BOX;

    // Create simulation system
    pS = makeSystem(rc, dt, alpha, sigma, d, z);

    printf("pS->dt = %f\n", pS->dt);

    // Set up orthographic projection
    setZoomForSystem();
    gluOrtho2D(zoomLeft, zoomRight, zoomBottom, zoomTop);

    // Print system information
    printf("System created:\n");
    printf("  Particles: %d\n", N);
    printf("  Cells: %dx%d\n", pS->nCells, pS->nCells);
    printf("  Cell size: %.2f\n", pS->cellSize);
    printf("  Cutoff radius: %.2f\n", pS->rc);
}


// Draw a filled circle at (x,y) with given radius
void drawCircle(float x, float y, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);  // Circle center
    int numSegments = NUM_SEGMENTS;
    for (int i = 0; i <= numSegments; i++){
        float theta = i * (2.0f * M_PI / numSegments);
        float dx = radius * cos(theta);
        float dy = radius * sin(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}


// Draw the spatial partitioning cell grid
void drawCellGrid() {
    glColor3f(0.7f, 0.7f, 0.7f);  // Light gray
    glLineWidth(0.5f);

    int nCells = pS->nCells;
    double cellSize = pS->cellSize;

    glBegin(GL_LINES);
    
    // Vertical lines
    for (int i = 0; i <= nCells; i++) {
        double x = i * cellSize;
        glVertex2d(x, 0.0);
        glVertex2d(x, nCells * cellSize);
    }
    
    // Horizontal lines
    for (int j = 0; j <= nCells; j++) {
        double y = j * cellSize;
        glVertex2d(0.0, y);
        glVertex2d(nCells * cellSize, y);
    }
    
    glEnd();
    glLineWidth(1.0f);
}


// Draw all particles as colored circles (red=infected, blue=susceptible)
void drawParticlesAsCircles() {
    int d = pS->d;
    double L = L_BOX;
    double r = PARTICLE_RADIUS;
    
    for (int i = 0; i < N; i++) {
        double xc = pS->x[d * i + 0];
        double yc = pS->x[d * i + 1];
        int state = pS->state[i];
        
        if (state == 0)
            glColor3f(1.0f, 0.2f, 0.2f);  // Red for infected
        else
            glColor3f(0.2f, 0.2f, 1.0f);  // Blue for susceptible
        
        // Draw particle at current position (no ghost copies for PBC)
        drawCircle((float)xc, (float)yc, (float)r);
    }
}


// Display function: render the simulation
void display_v00() {

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw cell grid if enabled
    if (gridView) {
        drawCellGrid();
    }

    // Draw particles
    drawParticlesAsCircles();

    glutSwapBuffers();
}


// Main display function with view mode switching
void display() {
    switch (viewMode) {
        case 0:
            display_v00();
            break;
        default:
            fprintf(stderr, "Warning: Unknown viewMode = %d\n", viewMode);
            break;
    }
}


// Handle window resizing and adjust viewport/projection
void reshape(int w, int h) {

    // Update window dimensions
    windowWidth = w;
    windowHeight = h;

    // Set viewport to cover entire window
    glViewport(0, 0, windowWidth, windowHeight);

    // Update projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set orthographic projection with current zoom settings
    gluOrtho2D(zoomLeft, zoomRight, zoomBottom, zoomTop);

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Request redraw
    glutPostRedisplay();
}


// Zoom in/out around a center point
void adjustZoom(double centerX, double centerY, double scaleFactor) {
    double width = zoomRight - zoomLeft;
    double height = zoomTop - zoomBottom;

    // Recalculate zoom boundaries
    zoomLeft   = centerX - (width / 2) * scaleFactor;
    zoomRight  = centerX + (width / 2) * scaleFactor;
    zoomBottom = centerY - (height / 2) * scaleFactor;
    zoomTop    = centerY + (height / 2) * scaleFactor;

    reshape(windowWidth, windowHeight);
}


// Reset zoom to show entire simulation box
void resetZoom() {
    setZoomForSystem();
    reshape(windowWidth, windowHeight);
}


// Pan the view in a given direction
void adjustZoomInDirection(double deltaX, double deltaY) {
    zoomTop += deltaY;
    zoomBottom += deltaY;
    zoomLeft += deltaX;
    zoomRight += deltaX;
    reshape(windowWidth, windowHeight);
}


// Timer callback: update simulation and request redraw
void update(int value){
    // Update simulation if not paused
    if(!pause){
       iteration(pS);  // Update particle positions
       
       // Choose propagation model (uncomment one)
       // propagation_v00(pS, 1.0, 0.5);  // Independent transitions
       // propagation_v01(pS, 1.0, 0.5);  // Count-based infection
       propagation_v02(pS, BETA, LAMBDA);     // Distance-dependent infection
    }

    // Request redraw
    glutPostRedisplay();

    // Schedule next update
    glutTimerFunc(1000 / FPS, update, 0);
}


// Handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC: exit program
            destroySystem(pS);
            exit(0);
            break;

        case '+': // Zoom in
            adjustZoom((zoomLeft + zoomRight) / 2, (zoomBottom + zoomTop) / 2, 1.0f / zoomFactor);
            break;

        case '-': // Zoom out
            adjustZoom((zoomLeft + zoomRight) / 2, (zoomBottom + zoomTop) / 2, zoomFactor);
            break;

        case 'r': // Reset zoom
            resetZoom();
            break;

        case 'v': case 'V': // Cycle view modes forward
            viewMode = (viewMode + 1) % nView;
            break;

        case 'b': case 'B': // Cycle view modes backward
            viewMode = (viewMode - 1 + nView) % nView;
            break;

        case 'f': // Increase FPS
            FPS++;
            printf("Frames per second: %d\n", FPS);
            break;

        case 'F': // Decrease FPS
            if (FPS > 1) {
                FPS--;
                printf("Frames per second: %d\n", FPS);
            } else {
                printf("Minimum FPS reached.\n");
            }
            break;

        case 'p': case 'P': // Pause/resume
            pause = !pause;
            break;

        case 'g': case 'G': // Toggle grid view
            gridView = !gridView;
            break;

        default:
            break;
    }
}


// Handle arrow key input for panning
void specialKeys(int key, int x, int y) {
    double pan_distance = 5.0;
    switch (key) {
        case GLUT_KEY_UP:
            adjustZoomInDirection(0, -pan_distance);
            break;
        case GLUT_KEY_DOWN:
            adjustZoomInDirection(0, pan_distance);
            break;
        case GLUT_KEY_LEFT:
            adjustZoomInDirection(-pan_distance, 0);
            break;
        case GLUT_KEY_RIGHT:
            adjustZoomInDirection(pan_distance, 0);
            break;
        default:
            break;
    }
}


// Main function: initialize and start simulation
int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Particle Simulation - SIS Epidemic Model");

    initOpenGL(argc, argv);

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(1000 / FPS, update, 0);

    // Print controls
    printf("\n=== CONTROLS ===\n");
    printf("ESC: Exit\n");
    printf("+/-: Zoom in/out\n");
    printf("r: Reset zoom\n");
    printf("p: Pause/resume\n");
    printf("g: Toggle grid view\n");
    printf("f/F: Increase/decrease FPS\n");
    printf("Arrows: Pan view\n");
    printf("\n");

    glutMainLoop();

    return 0;
}