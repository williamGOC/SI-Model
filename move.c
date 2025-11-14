#include "config.h"
#include "random.h"
#include "system.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <string.h>
#include <math.h>

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

// UI Control panel
GLint showUI = 1;    // Show/hide UI panel

// Button structure
typedef struct {
    float x, y, width, height;
    char label[32];
    int paramType; // 0=alpha, 1=lambda, 2=beta, 3=sigma
    int isIncrement; // 1 for +, 0 for -
} Button;

#define NUM_BUTTONS 8
Button buttons[NUM_BUTTONS];

// Parameter adjustment steps
double alphaStep = 0.05;
double lambdaStep = 0.05;
double betaStep = 0.1;
double sigmaStep = 0.1;

// Current parameter values (will be synced with system)
double currentAlpha = ALPHA;
double currentLambda = LAMBDA;
double currentBeta = BETA;
double currentSigma = SIGMA;


// Initialize UI buttons
void initButtons() {
    float startX = 15.0f;
    float startY = 20.0f;
    float buttonWidth = 30.0f;
    float buttonHeight = 25.0f;
    float spacing = 5.0f;
    float rowSpacing = 35.0f;
    
    // Alpha buttons (row 1)
    buttons[0] = (Button){startX, startY, buttonWidth, buttonHeight, "A-", 0, 0};
    buttons[1] = (Button){startX + buttonWidth + spacing, startY, buttonWidth, buttonHeight, "A+", 0, 1};
    
    // Lambda buttons (row 2)
    buttons[2] = (Button){startX, startY + rowSpacing, buttonWidth, buttonHeight, "L-", 1, 0};
    buttons[3] = (Button){startX + buttonWidth + spacing, startY + rowSpacing, buttonWidth, buttonHeight, "L+", 1, 1};
    
    // Beta buttons (row 3)
    buttons[4] = (Button){startX, startY + 2*rowSpacing, buttonWidth, buttonHeight, "B-", 2, 0};
    buttons[5] = (Button){startX + buttonWidth + spacing, startY + 2*rowSpacing, buttonWidth, buttonHeight, "B+", 2, 1};
    
    // Sigma buttons (row 4)
    buttons[6] = (Button){startX, startY + 3*rowSpacing, buttonWidth, buttonHeight, "S-", 3, 0};
    buttons[7] = (Button){startX + buttonWidth + spacing, startY + 3*rowSpacing, buttonWidth, buttonHeight, "S+", 3, 1};
}


// Draw a button
void drawButton(Button* btn, int isHovered) {
    // Button background
    if (isHovered)
        glColor3f(0.3f, 0.6f, 0.9f);  // Highlighted
    else
        glColor3f(0.2f, 0.4f, 0.7f);  // Normal
    
    glBegin(GL_QUADS);
    glVertex2f(btn->x, btn->y);
    glVertex2f(btn->x + btn->width, btn->y);
    glVertex2f(btn->x + btn->width, btn->y + btn->height);
    glVertex2f(btn->x, btn->y + btn->height);
    glEnd();
    
    // Button border
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn->x, btn->y);
    glVertex2f(btn->x + btn->width, btn->y);
    glVertex2f(btn->x + btn->width, btn->y + btn->height);
    glVertex2f(btn->x, btn->y + btn->height);
    glEnd();
    glLineWidth(1.0f);
    
    // Button text
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(btn->x + btn->width * 0.15f, btn->y + btn->height * 0.7f);
    for (char* c = btn->label; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}


// Draw UI panel with parameter values
void drawUI() {
    if (!showUI) return;
    
    // Save current projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);  // Screen coordinates (top-left origin)
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Draw semi-transparent background panel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.9f, 0.9f, 0.9f, 0.85f);  // Light gray with transparency
    
    float panelX = 10.0f;
    float panelY = 10.0f;
    float panelWidth = 220.0f;  // Increased width to accommodate spacing
    float panelHeight = 160.0f;
    
    glBegin(GL_QUADS);
    glVertex2f(panelX, panelY);
    glVertex2f(panelX + panelWidth, panelY);
    glVertex2f(panelX + panelWidth, panelY + panelHeight);
    glVertex2f(panelX, panelY + panelHeight);
    glEnd();
    
    // Draw panel border
    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panelX, panelY);
    glVertex2f(panelX + panelWidth, panelY);
    glVertex2f(panelX + panelWidth, panelY + panelHeight);
    glVertex2f(panelX, panelY + panelHeight);
    glEnd();
    glLineWidth(1.0f);
    
    glDisable(GL_BLEND);
    
    // Draw buttons
    for (int i = 0; i < NUM_BUTTONS; i++) {
        drawButton(&buttons[i], 0);
    }
    
    // Draw parameter values with more spacing from buttons
    char text[128];
    glColor3f(0.0f, 0.0f, 0.0f);
    
    float textX = 115.0f;  // Moved much further right (was 95.0f)
    float startY = 33.0f;
    float rowSpacing = 35.0f;
    
    // Alpha
    glRasterPos2f(textX, startY);
    snprintf(text, sizeof(text), "Alpha: %.3f", currentAlpha);
    for (char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Lambda
    glRasterPos2f(textX, startY + rowSpacing);
    snprintf(text, sizeof(text), "Lambda: %.3f", currentLambda);
    for (char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Beta
    glRasterPos2f(textX, startY + 2*rowSpacing);
    snprintf(text, sizeof(text), "Beta: %.3f", currentBeta);
    for (char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Sigma
    glRasterPos2f(textX, startY + 3*rowSpacing);
    snprintf(text, sizeof(text), "Sigma: %.3f", currentSigma);
    for (char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    // Restore previous matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


// Update parameter in the system
void updateParameter(int paramType, int isIncrement) {
    double step;
    double* param;
    char* paramName;
    
    switch (paramType) {
        case 0: // Alpha
            param = &currentAlpha;
            step = alphaStep;
            paramName = "Alpha";
            break;
        case 1: // Lambda
            param = &currentLambda;
            step = lambdaStep;
            paramName = "Lambda";
            break;
        case 2: // Beta
            param = &currentBeta;
            step = betaStep;
            paramName = "Beta";
            break;
        case 3: // Sigma
            param = &currentSigma;
            step = sigmaStep;
            paramName = "Sigma";
            break;
        default:
            return;
    }
    
    // Update parameter value
    if (isIncrement) {
        *param += step;
    } else {
        *param -= step;
        if (*param < 0.0) *param = 0.0; // Prevent negative values
    }
    
    // Update system parameters (if applicable)
    if (paramType == 0 && pS != NULL) {
        pS->alpha = currentAlpha;
    } else if (paramType == 3 && pS != NULL) {
        pS->sigma = currentSigma;
    }
    
    printf("%s updated to: %.3f\n", paramName, *param);
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

    // Sync current parameters with system
    currentAlpha = pS->alpha;
    currentSigma = pS->sigma;
    currentBeta = BETA;
    currentLambda = LAMBDA;

    printf("pS->dt = %f\n", pS->dt);

    // Set up orthographic projection
    setZoomForSystem();
    gluOrtho2D(zoomLeft, zoomRight, zoomBottom, zoomTop);

    // Initialize UI buttons
    initButtons();

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

    // Draw UI on top
    drawUI();

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

    // Reinitialize buttons with new window size
    initButtons();

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
       propagation_v02(pS, currentBeta, currentLambda);  // Distance-dependent infection (using current values)
    }

    // Request redraw
    glutPostRedisplay();

    // Schedule next update
    glutTimerFunc(1000 / FPS, update, 0);
}


// Handle mouse clicks
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Check if click is on any button
        for (int i = 0; i < NUM_BUTTONS; i++) {
            Button* btn = &buttons[i];
            if (x >= btn->x && x <= btn->x + btn->width &&
                y >= btn->y && y <= btn->y + btn->height) {
                updateParameter(btn->paramType, btn->isIncrement);
                glutPostRedisplay();
                return;
            }
        }
        
        // If not on a button, check if clicking near a particle to infect it
        // Convert screen coordinates to simulation coordinates
        double simX = zoomLeft + ((double)x / windowWidth) * (zoomRight - zoomLeft);
        double simY = zoomTop - ((double)y / windowHeight) * (zoomTop - zoomBottom);
        
        // Find closest particle within click radius
        int d = pS->d;
        double clickRadius = 1.0;  // Detection radius in simulation units
        double minDist = clickRadius;
        int closestParticle = -1;
        
        for (int i = 0; i < N; i++) {
            double px = pS->x[d * i + 0];
            double py = pS->x[d * i + 1];
            
            // Calculate distance to click
            double dx = px - simX;
            double dy = py - simY;
            double dist = sqrt(dx*dx + dy*dy);
            
            if (dist < minDist) {
                minDist = dist;
                closestParticle = i;
            }
        }
        
        // Infect the closest particle if found
        if (closestParticle >= 0) {
            pS->state[closestParticle] = 0;  // 0 = infected
            printf("Particle %d infected! (distance: %.2f)\n", closestParticle, minDist);
            glutPostRedisplay();
        }
    }
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

        case 'u': case 'U': // Toggle UI panel
            showUI = !showUI;
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
    glutCreateWindow("SIS Epidemic Model");

    initOpenGL();

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutTimerFunc(1000 / FPS, update, 0);

    // Print controls
    printf("\n=== CONTROLS ===\n");
    printf("ESC: Exit\n");
    printf("+/-: Zoom in/out\n");
    printf("r: Reset zoom\n");
    printf("p: Pause/resume\n");
    printf("g: Toggle grid view\n");
    printf("u: Toggle UI panel\n");
    printf("f/F: Increase/decrease FPS\n");
    printf("Arrows: Pan view\n");
    printf("\nUI BUTTONS:\n");
    printf("A-/A+: Decrease/Increase Alpha\n");
    printf("L-/L+: Decrease/Increase Lambda\n");
    printf("B-/B+: Decrease/Increase Beta\n");
    printf("S-/S+: Decrease/Increase Sigma\n");
    printf("\nMOUSE:\n");
    printf("Left-click on buttons: Adjust parameters\n");
    printf("Left-click near particle: Infect particle\n");
    printf("\n");

    glutMainLoop();

    return 0;
}