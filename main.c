#include "config.h"
#include "random.h"
#include "system.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

// Dimensión de las partículas en la visualización
#define PARTICLE_RADIUS 0.3

// Número de segmentos para dibujar círculos
#define NUM_SEGMENTS 8

systemSI *pS = NULL;

// Initial zoom settings for orthographic projection and zoom factor
double zoomLeft;
double zoomRight;
double zoomBottom;
double zoomTop;

GLfloat zoomFactor = 1.1;

// Default window dimensions
GLint windowWidth  = 800;
GLint windowHeight = 800;

// System global paramenters
GLint FPS = 20;     // Frames per second
GLint viewMode = 0; // Toggle between display modes
GLint pause = 1;    // Pause simulation
GLint nView = 1;    // Number of views


// Set zoom based on the network type
void setZoomForSystem() {

    zoomLeft   = 0.0;
    zoomRight  = L_BOX;
    zoomBottom = 0;
    zoomTop    = L_BOX;
}

void initOpenGL() {

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    srand(time(NULL));
    seed_random(0);
    
    double rc = RC;
    double dt = 0.1;
    double alpha = 0.5;
    double sigma = 5.1;
    double d = 2;
    double z = 9;

    int nCells = L_BOX;

    // TODO
    pS = makeSystem(rc, dt, alpha, sigma, d, z);
    setZoomForSystem();

    gluOrtho2D(zoomLeft, zoomRight, zoomBottom, zoomTop);

    printf("Sistema creado:\n");
    printf("  Partículas: %d\n", N);
    printf("  Celdas: %dx%d\n", pS->nCells, pS->nCells);
    printf("  Tamaño de celda: %.2f\n", pS->cellSize);
    printf("  Radio de corte: %.2f\n", pS->rc);
}


// Función auxiliar para dibujar un círculo
void drawCircle(float x, float y, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);                                  // Centro del círculo
    int numSegments = NUM_SEGMENTS;
    for (int i = 0; i <= numSegments; i++){
        float theta = i * (2.0f * M_PI / numSegments); // Radianes
        float dx = radius * cos(theta);
        float dy = radius * sin(theta);
        glVertex2f(x + dx, y + dy);                    // Vértices del círculo
    }
    glEnd();
}


// Función para dibujar la grilla de celdas
void drawCellGrid() {
    glColor3f(0.7f, 0.7f, 0.7f);  // Color gris claro
    glLineWidth(0.5f);

    int nCells = pS->nCells;
    double cellSize = pS->cellSize;

    glBegin(GL_LINES);
    
    // Líneas verticales
    for (int i = 0; i <= nCells; i++) {
        double x = i * cellSize;
        glVertex2d(x, 0.0);
        glVertex2d(x, nCells * cellSize);
    }
    
    // Líneas horizontales
    for (int j = 0; j <= nCells; j++) {
        double y = j * cellSize;
        glVertex2d(0.0, y);
        glVertex2d(nCells * cellSize, y);
    }
    
    glEnd();
    glLineWidth(1.0f);
}


// Función para dibujar todas las partículas
void drawParticles() {
    
    int d = pS->d;
    double cellSize = pS->cellSize;
    int nCells = pS->nCells;

    glPointSize(2.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i < N; i++) {
        double x = pS->x[d * i + 0];
        double y = pS->x[d * i + 1];

        // Asignar color según el estado
        // Estado 0 = Rojo (infectado/activo)
        // Estado 1 = Azul (susceptible)
        int state = pS->state[i];
        
        if (state == 0) {
            glColor3f(1.0f, 0.0f, 0.0f);  // Rojo
        } else {
            glColor3f(0.0f, 0.0f, 1.0f);  // Azul
        }

        glVertex2d(x, y);
    }
    
    glEnd();
    glPointSize(1.0f);
}


// Función para dibujar partículas como círculos (más lento pero mejor visualización)
void drawParticlesAsCircles() {
    
    int d = pS->d;

    for (int i = 0; i < N; i++) {
        double x = pS->x[d * i + 0];
        double y = pS->x[d * i + 1];
        int state = pS->state[i];

        // Asignar color según el estado
        if (state == 0) {
            glColor3f(1.0f, 0.2f, 0.2f);  // Rojo brillante
        } else {
            glColor3f(0.2f, 0.2f, 1.0f);  // Azul
        }

        drawCircle(x, y, PARTICLE_RADIUS);
    }
}


// Función para mostrar información estadística en consola
void printStatistics() {
    int count_state_0 = 0;
    int count_state_1 = 0;

    for (int i = 0; i < N; i++) {
        if (pS->state[i] == 0) {
            count_state_0++;
        } else {
            count_state_1++;
        }
    }

    printf("\n=== ESTADÍSTICAS ===\n");
    printf("Partículas estado 0 (rojo): %d (%.1f%%)\n", 
           count_state_0, 100.0 * count_state_0 / N);
    printf("Partículas estado 1 (azul): %d (%.1f%%)\n", 
           count_state_1, 100.0 * count_state_1 / N);
}


// Función principal de visualización: modo 0
void display_v00() {
    
    // Limpiar el buffer de color y profundidad
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Dibujar grilla de celdas (opcional)
    drawCellGrid();

    // Dibujar partículas como puntos (rápido)
    // drawParticles();

    // O dibujar partículas como círculos (más lento pero más visible)
    drawParticlesAsCircles();

    // Dibujar legend (texto)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 0.2f, 0.2f);
    glRasterPos2f(5, 95);
    // Nota: glutBitmapCharacter requeriría más código para texto
    // Por ahora simplemente omitimos la leyenda

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Intercambiar buffers para mostrar el resultado
    glutSwapBuffers();
}



// Main display function toggling between different view modes
void display() {
    switch (viewMode) {
        case 0:
            display_v00();
            break;
        default:
            // Opcional: manejo de casos inesperados
            fprintf(stderr, "Warning: Unknown viewMode = %d\n", viewMode);
            break;
    }
}

// Adjusts the viewport and sets the orthographic projection whenever the window is resized
void reshape(int w, int h) {

    // Update the window dimensions
    windowWidth = w;
    windowHeight = h;

    // Set the new viewport to cover the entire window
    glViewport(0, 0, windowWidth, windowHeight);

    // Switch to the projection matrix to define the camera view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset the projection matrix to avoid accumulation of transformations

    // Set the orthographic projection based on the zoom variables
    // This defines the left, right, bottom, and top clipping planes for the 2D view
    gluOrtho2D(zoomLeft, zoomRight, zoomBottom, zoomTop);

    // Switch back to the model-view matrix for object transformations
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset the model-view matrix

    // Request a redraw of the window to apply the new projection and viewport settings
    glutPostRedisplay();
}


// Adjust zoom level and update the orthographic projection based on the mouse interaction
void adjustZoom(double centerX, double centerY, double scaleFactor) {
    // Calculate the width and height of the current zoomed region
    double width = zoomRight - zoomLeft;
    double height = zoomTop - zoomBottom;

    // Recalculate the zoom region around the center point with the new scale factor
    zoomLeft   = centerX - (width / 2) * scaleFactor;   // New left boundary of the zoom
    zoomRight  = centerX + (width / 2) * scaleFactor;   // New right boundary of the zoom
    zoomBottom = centerY - (height / 2) * scaleFactor;  // New bottom boundary of the zoom
    zoomTop    = centerY + (height / 2) * scaleFactor;  // New top boundary of the zoom

    // Call reshape function to update the orthographic projection and redraw the scene
    reshape(windowWidth, windowHeight);
}


// Reset the zoom level to the default initial view
void resetZoom() {
    // Define the initial zoom boundaries (scaled to the appropriate size of the grid)
    setZoomForSystem();

    // Call reshape function to update the projection and redraw the scene
    reshape(windowWidth, windowHeight);
}


// Function to adjust zoom in a specific direction (for 8, 2, 4, 6 keys)
void adjustZoomInDirection(double deltaX, double deltaY) {
    zoomTop += deltaY;
    zoomBottom += deltaY;
    zoomLeft += deltaX;
    zoomRight += deltaX;
    reshape(windowWidth, windowHeight);  // Update the projection
}


// Timer-based update function for animation
void update(int value){
    // Perform a step in the pN simulation, if the simulation is not paused
    if(!pause){
       iteration(pS);
       getCellIndex(pS);

       // DEBUG: Imprime posición de primera partícula cada 20 frames
       static int frame_count = 0;
       if (frame_count++ % 20 == 0) {
           printf("Partícula 0: x=%.2f, y=%.2f\n", pS->x[0], pS->x[1]);
       }
    }

    // Request the redraw of the scene by posting a redisplay event to GLUT
    glutPostRedisplay();

    // Set up the timer to call the update function again after a delay,
    // maintaining the animation frame rate (FPS frames per second)
    glutTimerFunc(1000 / FPS, update, 0);
}


/// Keyboard controls for user interactions
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // ESC key: Exit the program
            destroySystem(pS); // Free network memory
            exit(0);
            break;

        case '+': // Zoom in
            adjustZoom((zoomLeft + zoomRight) / 2, (zoomBottom + zoomTop) / 2, 1.0f / zoomFactor);
            break;

        case '-': // Zoom out
            adjustZoom((zoomLeft + zoomRight) / 2, (zoomBottom + zoomTop) / 2, zoomFactor);
            break;

        case 'r': // Reset zoom to default
            resetZoom();
            break;

        case 'v': case 'V': // Cycle forward through view modes
            viewMode = (viewMode + 1) % nView;
            break;

        case 'b': case 'B': // Cycle backward through view modes
            viewMode = (viewMode - 1 + nView) % nView;
            break;

        case 'f': // Increase FPS
            FPS++;
            printf("Frames per second: %d\n", FPS);
            break;

        case 'F': // Decrease FPS (minimum = 1)
            if (FPS > 1) {
                FPS--;
                printf("Frames per second: %d\n", FPS);
            } else {
                printf("Minimum FPS reached.\n");
            }
            break;

        //case 't': // Print current simulation time
        //    printf("Current time step: %lld\n", pN->time);
        //    break;

        // case 's': case 'S': // Save screenshot (BMP)
        //    savingGIF = !savingGIF;
        //    break;

        case 'p': case 'P': // Pause/resume simulation
            pause = !pause;
            break;
        
        //case 'n': // tecla 'n' avanza un paso si está en pausa
        //    if(pause){
        //        mcStep(pN);              // un único paso
        //        glutPostRedisplay();   // forzar redibujado
        //    }
        //    break;
        //case 'g': case 'G':
        //    minimiza_CG(pN);
        //    break;
        
        //case 'k': case 'K':
        //    defor = !defor;
        //    break;
        
        //case 'a': case 'A':
        //    onlyAffin = !onlyAffin;
        //    break;

        default:
            break;
    }
}



// Arrow key controls
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

int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Particle Simulation - Press 'h' for help");

    initOpenGL(argc, argv);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(1000 / FPS, update, 0);

    printf("\n=== CONTROLES ===\n");
    printf("ESC: Salir\n");
    printf("+/-: Zoom in/out\n");
    printf("r: Reset zoom\n");
    printf("p: Pausa/resume\n");
    printf("f/F: Aumentar/disminuir FPS\n");
    printf("s: Mostrar estadísticas\n");
    printf("Flechas: Desplazar vista\n");
    printf("\n");

    glutMainLoop();

    return 0;
}
