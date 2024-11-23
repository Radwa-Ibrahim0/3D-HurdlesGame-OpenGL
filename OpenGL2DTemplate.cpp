#include <stdlib.h>
#include <glut.h>
#include <vector>
#include <random>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#define NOMINMAX
#include <iostream>
#include <algorithm>
#include <cmath>
#include <Windows.h>
#include <mmsystem.h>
#include <fstream>
#pragma comment(lib, "winmm.lib")

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Function prototypes
void display();
void init();
void reshape(int w, int h);
void idle();
void keyboard(unsigned char key, int x, int y);
void drawTrack();
void drawWalls();
void drawPlayer();
void drawHurdles();
void drawStadium();
void drawStartFinishLines();
void drawWinningBanner();
void drawFlags();
void drawGreenSpace();
void drawBush(float x, float y, float z);
void drawFlower(float x, float y, float z);
void drawBenchArea();
void drawCylinder(GLfloat radius, GLfloat height, GLint slices, GLint stacks);
void updateWallColor();
void updatePlayerPosition();
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);
void drawHUD();
void checkHurdleCollision();
void checkFinishLine();
void playSoundEffect(const char* soundFile);
void playFlagRotateSound();
void playWinSound();
void playLoseSound();
void playJumpSound();
void playCheerSound();
void playBoundarySound();
void playFlowerGrowSound();
void playCollision();
void updateAudio();
void audioManager();
void moveCameraInCurrentView(float dx, float dy, float dz);
void updateCameraPosition();

// Global variables for camera
float cameraX = 0.0f, cameraY = 3.0f, cameraZ = -50.0f;
int cameraView = 0; // 0: Free, 1: Top, 2: Side, 3: Front
float cameraRotationY = 0.0f;
float cameraDistance = 50.0f;
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;

// Player variables
float playerX = 0.0f, playerY = 2.1f, playerZ = -47.0f;
bool playerMoving = false;
bool isMovingForward = false;
float playerRotation = 0.0f;
bool isJumping = false;
float jumpVelocity = 0.0f;
float jumpForwardVelocity = 0.0f;
bool isMoving = false;
int lastPressedKey = 0;

// Track dimensions
const float trackWidth = 20.0f;
const float trackLength = 100.0f;
float bannerZ = trackLength / 2 - 5.0f;

// Area dimensions
const float brownAreaWidth = 5.0f;
const float greenAreaWidth = 5.0f;
const float stadiumWidth = 15.0f;
const float wallThickness = 1.0f;

// Animation variables
bool flagsRotating = false;
float flagRotation = 0.0f;
bool flowersGrowing = false;
float flowerScale = 1.0f;
float wallColorTimer = 0.0f;
float wallColor[3] = { 0.7f, 0.7f, 0.7f };
bool peopleCheeringInStadium = false;
float cheeringOffset = 0.0f;
bool bannerPulsing = true;
float bannerHeight = 2.0f;
bool benchItemsChangingColor = false;
float benchItemsColor[3] = { 0.6f, 0.4f, 0.2f }; // Original bench color
bool hurdlesChangingColor = false;
int hurdleColorIndex = 0;
float hurdleColors[3][3] = {
    {1.0f, 1.0f, 1.0f}, // White (original color)
    {0.5f, 0.5f, 0.7f}, // Muted blue
    {0.7f, 0.5f, 0.5f}  // Muted red
};
float benchColor[3] = { 0.7f, 0.7f, 0.7f }; // Gray color for the bench
bool bushesRotating = false;
float bushRotation = 0.0f;

// Game flow variables
float gameTime = 30.0f; 
bool gameOver = false;
bool playerWon = false;

// Sound Variables
bool isSoundEffectPlaying = false;
DWORD lastPlayTime = 0;

void drawCylinder(GLfloat radius, GLfloat height, GLint slices, GLint stacks) {
    GLUquadricObj* quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluCylinder(quadric, radius, radius, height, slices, stacks);
    gluDisk(quadric, 0.0, radius, slices, 1);
    glTranslatef(0.0, 0.0, height);
    gluDisk(quadric, 0.0, radius, slices, 1);
    glTranslatef(0.0, 0.0, -height);
    gluDeleteQuadric(quadric);
}

int main(int argc, char** argv) {
    /*if (true) {
        playCheerSound();
        std::cout << "Sound played successfully." << std::endl;
    }
    else {
        DWORD error = GetLastError();
        std::cerr << "PlaySound error: " << error << std::endl;
    }
    Sleep(3000);*/
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Hurdles Game");
    init();

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    audioManager();
    
    glutMainLoop();
    return 0;
}

void init() {
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue background
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
}

void moveCameraInCurrentView(float dx, float dy, float dz) {
    float speed = 0.5f;
    switch (cameraView) {
    case 0: // Free view
        cameraX += dx * cos(cameraAngleY) + dz * sin(cameraAngleY);
        cameraY += dy;
        cameraZ += -dx * sin(cameraAngleY) + dz * cos(cameraAngleY);
        break;
    case 1: // Top view
        cameraX += dx * speed;
        cameraZ += dy * speed;
        cameraDistance = std::max(5.0f, cameraDistance - dz * speed);
        break;
    case 2: // Side view
        cameraZ += dx * speed;
        cameraY += dy * speed;
        cameraX = std::max(-trackWidth / 2 - 5.0f, std::min(cameraX - dz * speed, trackWidth / 2 + 5.0f));
        break;
    case 3: // Front view
        cameraX += dx * speed;
        cameraY += dy * speed;
        cameraZ = std::max(-trackLength / 2 - 3.0f, std::min(cameraZ - dz * speed, trackLength / 2 + 3.0f));
        break;
    }
}

void updateCameraPosition() {
    if (cameraView == 1) { // Top view
        cameraX = playerX;
        cameraZ = playerZ;
    }
    else if (cameraView == 2) { // Side view
        cameraZ = playerZ;
    }
    else if (cameraView == 3) { // Front view
        cameraX = playerX;
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void idle() {
    updatePlayerPosition();
    audioManager();

    if (!gameOver) {
        gameTime -= 0.016f; // Assuming 60 FPS, adjust if needed
        if (gameTime <= 0) {
            gameOver = true;
            playerWon = false;
            playLoseSound();
        }
    }

    if (playerMoving) {
       /* playerZ += 0.1f;
        if (playerZ > 45.0f) playerZ = -47.0f;*/
        if (cameraView == 0) {
            cameraZ = playerZ - 3.0f;
        }
    }

    if (isJumping) {
        playerY += jumpVelocity;
        jumpVelocity -= 0.02f;  // Gravity

        if (playerY <= 2.1f) {  // Ground level
            playerY = 2.1f;
            isJumping = false;
        }
    }

    if (flagsRotating) {
        flagRotation += 1.0f;
        if (flagRotation >= 360.0f) flagRotation = 0.0f;
    }

    if (peopleCheeringInStadium) {
        cheeringOffset = 0.1f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f);
    }
    else {
        cheeringOffset = 0.0f;
    }

    if (flowersGrowing) {
        flowerScale = 1.0f + 0.2f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f);
    }

    bannerHeight = 2.0f + 0.5f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f);

    if (benchItemsChangingColor) {
        benchItemsColor[0] = 0.6f + 0.4f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f);
        benchItemsColor[1] = 0.4f + 0.4f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.002f);
        benchItemsColor[2] = 0.2f + 0.4f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    }

    if (hurdlesChangingColor) {
        static float colorChangeTimer = 0.0f;
        colorChangeTimer += 0.01f;
        if (colorChangeTimer >= 1.0f) {
            colorChangeTimer = 0.0f;
            hurdleColorIndex = (hurdleColorIndex + 1) % 3; // Cycle through 3 colors
            glutPostRedisplay();
        }
    }
    if (bushesRotating) {
        bushRotation += 1.0f;
        if (bushRotation >= 360.0f) {
            bushRotation -= 360.0f;
        }
    }

    updateWallColor();

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    if (!gameOver) {
        float dx = 0.0f, dy = 0.0f, dz = 0.0f;
        switch (key) {
        case 'w': case 'W':
            dy = (cameraView == 1) ? -1.0f : 1.0f;
            break;
        case 's': case 'S':
            dy = (cameraView == 1) ? 1.0f : -1.0f;
            break;
        case 'a': case 'A':
            dx = 1.0f;
            break;
        case 'd': case 'D':
            dx = -1.0f;
            break;
        case 'q': case 'Q':
            dz = 1.0f;
            break;
        case 'e': case 'E':
            dz = -1.0f;
            break;
        case '0':
            cameraView = 0;
            cameraX = 0.0f; cameraY = 3.0f; cameraZ = -50.0f;
            cameraAngleX = 0.0f; cameraAngleY = 0.0f;
            break;
        case '1':
            cameraView = 1;
            cameraX = playerX; cameraY = 50.0f; cameraZ = playerZ;
            cameraDistance = 50.0f;
            break;
        case '2':
            cameraView = 2;
            cameraX = -2.0f; // Inside the left wall
            cameraY = 3.0f;
            cameraZ = playerZ;
            break;
        case '3':
            cameraView = 3;
            cameraX = playerX;
            cameraY = 3.0f; // Eye level
            cameraZ = trackLength / 2 - 7.0f; // Inside the back wall
            cameraDistance = 20.0f;
            break;
        case 27: exit(0); break; // ESC key
        case 'z': case 'Z': 
            flagsRotating = !flagsRotating;
            if (flagsRotating) playFlagRotateSound();
            break;
        case 'x': case 'X': 
            peopleCheeringInStadium = !peopleCheeringInStadium; 
            if (peopleCheeringInStadium) playCheerSound();
            break;
        case 'c': case 'C':
            flowersGrowing = !flowersGrowing;
            if (!flowersGrowing) flowerScale = 1.0f;
            if (flowersGrowing) playFlowerGrowSound();
            break;
        case 'v': case 'V':
            benchItemsChangingColor = !benchItemsChangingColor;
            if (!benchItemsChangingColor) {
                benchItemsColor[0] = 0.6f;
                benchItemsColor[1] = 0.4f;
                benchItemsColor[2] = 0.2f;
            }
            break;
        case 'b': case 'B':
            hurdlesChangingColor = !hurdlesChangingColor;
            if (!hurdlesChangingColor) {
                hurdleColorIndex = 0; // Reset to white when stopping
            }
            break;
        case 'n': case 'N': bushesRotating = !bushesRotating; break;
        case ' ':
            if (!isJumping) {
                isJumping = true;
                jumpVelocity = 0.5f;
                jumpForwardVelocity = 0.2f;  // Add forward velocity to the jump
                playJumpSound();
            }
            break;
        }
        moveCameraInCurrentView(dx, dy, dz);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (!gameOver) {  // Only allow movement if the game is not over
        float moveSpeed = 0.1f;
        float rotateSpeed = 2.0f;

        switch (key) {
        case GLUT_KEY_UP:
            playerX += moveSpeed * sin(playerRotation * M_PI / 180.0f);
            playerZ += moveSpeed * cos(playerRotation * M_PI / 180.0f);
            playerMoving = true;
            isMovingForward = true;
            break;
        case GLUT_KEY_DOWN:
            playerX -= moveSpeed * sin(playerRotation * M_PI / 180.0f);
            playerZ -= moveSpeed * cos(playerRotation * M_PI / 180.0f);
            playerMoving = true;
            isMovingForward = false;
            break;
        case GLUT_KEY_LEFT:
            playerRotation += rotateSpeed;
            if (playerRotation >= 360.0f) playerRotation -= 360.0f;
            break;
        case GLUT_KEY_RIGHT:
            playerRotation -= rotateSpeed;
            if (playerRotation < 0.0f) playerRotation += 360.0f;
            break;
        }

        // Add boundary checks
        if (playerX < -trackWidth / 2 + 1.0f) playerX = -trackWidth / 2 + 1.0f;
        if (playerX > trackWidth / 2 - 1.0f) playerX = trackWidth / 2 - 1.0f;
        if (playerZ < -trackLength / 2 + 1.0f) playerZ = -trackLength / 2 + 1.0f;
        if (playerZ > trackLength / 2 - 1.0f) playerZ = trackLength / 2 - 1.0f;

        glutPostRedisplay();
    }
}

void specialKeysUp(int key, int x, int y) {
    isMoving = false;
    playerMoving = false;
    if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN) {
        isMovingForward = false;
    }
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (gameOver) {
        audioManager();
        // Display black screen with game result
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);

        std::string resultText = playerWon ? "You Win!" : "You Lose!";
        if (playerWon)
            playWinSound();
        else
            playLoseSound();
        glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 50, glutGet(GLUT_WINDOW_HEIGHT) / 2 - 20);
        for (char c : resultText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        glEnable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    else {
        updateCameraPosition(); // Update camera position based on player movement

        switch (cameraView) {
        case 0: // Free view
            gluLookAt(cameraX, cameraY, cameraZ,
                cameraX + sin(cameraAngleY), cameraY + sin(cameraAngleX), cameraZ + cos(cameraAngleY),
                0.0, 1.0, 0.0);
            break;
        case 1: // Top view
            gluLookAt(cameraX, cameraDistance, cameraZ,
                cameraX, 0, cameraZ,
                0.0, 0.0, -1.0);
            break;
        case 2: // Side view
            gluLookAt(cameraX, cameraY, cameraZ,
                playerX, playerY, playerZ,
                0.0, 1.0, 0.0);
            break;
        case 3: // Front view
            gluLookAt(cameraX, cameraY, cameraZ,
                cameraX, cameraY, cameraZ - cameraDistance,
                0.0, 1.0, 0.0);
            break;
        }

        drawTrack();
        drawBenchArea();
        drawGreenSpace();
        drawStadium();
        drawWalls();
        drawPlayer();
        drawHurdles();
        drawStartFinishLines();
        drawWinningBanner();
        drawFlags();
        drawHUD();
    }

    glutSwapBuffers();
}

void drawTrack() {
    glPushMatrix();
    glColor3f(0.545f, 0.271f, 0.075f); // Brown color
    glTranslatef(0.0f, -0.1f, 0.0f);
    glScalef(trackWidth, 0.2f, trackLength);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw lane lines
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    for (int i = -3; i <= 3; i++) {
        glPushMatrix();
        glTranslatef(i * 2.5f, 0.01f, 0.0f);
        glScalef(0.1f, 0.1f, trackLength);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

void drawGreenSpace() {
    // Draw grass
    glColor3f(0.0f, 0.5f, 0.0f); // Green color
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * (trackWidth / 2 + brownAreaWidth + greenAreaWidth / 2), 0.0f, 0.0f);
        glScalef(greenAreaWidth, 0.1f, trackLength);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Draw bushes
    for (int side = -1; side <= 1; side += 2) {
        for (int i = -40; i <= 40; i += 8) {
            drawBush(side * (trackWidth / 2 + brownAreaWidth + greenAreaWidth / 2), 0.5f, (float)i);
        }
    }

    // Draw flowers
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-greenAreaWidth / 2, greenAreaWidth / 2);
    for (int side = -1; side <= 1; side += 2) {
        for (int i = -45; i <= 45; i += 5) {
            float offsetX = distribution(generator);
            float offsetZ = distribution(generator);
            drawFlower(side * (trackWidth / 2 + brownAreaWidth + greenAreaWidth / 2 + offsetX), 0.1f, i + offsetZ);
        }
    }
}

void drawStadium() {
    float levelHeight = 2.0f;
    float levelDepth = 3.0f;
    int numLevels = 5;

    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * (trackWidth / 2 + brownAreaWidth + greenAreaWidth), 0.0f, 0.0f);

        for (int level = 0; level < numLevels; level++) {
            glPushMatrix();
            glTranslatef(side * (level * levelDepth), level * levelHeight, 0.0f);

            // Stadium level
            glColor3f(0.7f, 0.7f, 0.7f);
            glPushMatrix();
            glScalef(levelDepth, levelHeight, trackLength);
            glutSolidCube(1.0);
            glPopMatrix();

            // Seats and people
            for (int i = -24; i <= 24; i += 2) {
                glPushMatrix();
                glTranslatef(side * (levelDepth / 2), levelHeight / 2 + cheeringOffset, (float)i * 2.0f);

                // Seat
                glColor3f(0.5f, 0.5f, 0.5f);
                glPushMatrix();
                glTranslatef(0.0f, -0.2f, 0.0f);
                glScalef(0.8f, 0.1f, 0.8f);
                glutSolidCube(1.0);
                glPopMatrix();

                // Person
                glPushMatrix();
                glTranslatef(side * -0.2f, 0.3f, 0.0f);

                // Body
                glColor3f(0.8f, 0.4f, 0.2f);
                glPushMatrix();
                glScalef(0.4f, 0.6f, 0.2f);
                glutSolidCube(1.0);
                glPopMatrix();

                // Head
                glTranslatef(0.0f, 0.5f, 0.0f);
                glColor3f(0.95f, 0.75f, 0.6f);
                glutSolidSphere(0.2, 10, 10);

                glPopMatrix();
                glPopMatrix();
            }

            glPopMatrix();
        }
        glPopMatrix();
    }
}

void drawWalls() {
    float totalWidth = trackWidth + 2 * (brownAreaWidth + greenAreaWidth + stadiumWidth);
    float wallHeight = 15.0f;
    float stadiumHeight = 5 * 2.0f;

    glColor3fv(wallColor);

    // Left wall
    glPushMatrix();
    glTranslatef(-(totalWidth / 2), wallHeight / 2, 0.0f);
    glScalef(wallThickness, wallHeight, trackLength);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right wall
    glPushMatrix();
    glTranslatef(totalWidth / 2, wallHeight / 2, 0.0f);
    glScalef(wallThickness, wallHeight, trackLength);
    glutSolidCube(1.0);
    glPopMatrix();

    // Back wall
    glPushMatrix();
    glTranslatef(0.0f, wallHeight / 2, trackLength / 2 + wallThickness / 2);
    glScalef(totalWidth, wallHeight, wallThickness);
    glutSolidCube(1.0);
    glPopMatrix();

    // Add spheres on top of the walls
    glColor3f(0.6f, 0.6f, 0.6f);
    for (int i = -49; i <= 49; i += 2) {
        glPushMatrix();
        glTranslatef(-(totalWidth / 2), wallHeight, (float)i);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(totalWidth / 2, wallHeight, (float)i);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();
    }
    for (float i = -totalWidth / 2; i <= totalWidth / 2; i += 2) {
        glPushMatrix();
        glTranslatef(i, wallHeight, trackLength / 2 + wallThickness / 2);
        glutSolidSphere(0.5, 10, 10);
        glPopMatrix();
    }
}

void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(playerRotation, 0.0f, 1.0f, 0.0f);

    // Head
    glColor3f(0.95f, 0.75f, 0.6f);
    glutSolidSphere(0.25, 20, 20);

    // Body
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glScalef(0.4f, 1.35f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Arms and legs animation
    float animationAngle = playerMoving ? 45 * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005) : 0;

    // Right Arm
    glColor3f(0.95f, 0.75f, 0.6f);
    glPushMatrix();
    glTranslatef(0.3f, -0.6f, 0.0f);
    glRotatef(animationAngle, 1.0f, 0.0f, 0.0f);
    glScalef(0.1f, 0.9f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Left Arm
    glPushMatrix();
    glTranslatef(-0.3f, -0.6f, 0.0f);
    glRotatef(-animationAngle, 1.0f, 0.0f, 0.0f);
    glScalef(0.1f, 0.9f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right Leg
    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.1f, -1.65f, 0.0f);
    glRotatef(animationAngle, 1.0f, 0.0f, 0.0f);
    glScalef(0.1f, 0.9f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Left Leg
    glPushMatrix();
    glTranslatef(-0.1f, -1.65f, 0.0f);
    glRotatef(-animationAngle, 1.0f, 0.0f, 0.0f);
    glScalef(0.1f, 0.9f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawHurdles() {
    float hurdleHeight = 1.05f;
    float hurdleWidth = trackWidth / 8.0f;
    float hurdleSpacing = hurdleWidth;  // Remove gaps between hurdles

    glColor3fv(hurdleColors[hurdleColorIndex]);

    for (int i = -35; i <= 35; i += 10) {
        glPushMatrix();
        glTranslatef(-trackWidth / 2 + hurdleWidth / 2, 0, (float)i);

        for (int j = 0; j < 8; j++) {
            glPushMatrix();
            glTranslatef(j * hurdleSpacing, hurdleHeight / 2, 0);

            // Base
            glPushMatrix();
            glScalef(hurdleWidth * 0.9f, 0.1f, 0.1f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Left pole
            glPushMatrix();
            glTranslatef(-hurdleWidth * 0.45f, hurdleHeight / 2, 0.0f);
            glScalef(0.05f, hurdleHeight, 0.05f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Right pole
            glPushMatrix();
            glTranslatef(hurdleWidth * 0.45f, hurdleHeight / 2, 0.0f);
            glScalef(0.05f, hurdleHeight, 0.05f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Top bar
            glPushMatrix();
            glTranslatef(0.0f, hurdleHeight, 0.0f);
            glScalef(hurdleWidth * 0.9f, 0.05f, 0.05f);
            glutSolidCube(1.0);
            glPopMatrix();

            glPopMatrix();
        }

        glPopMatrix();
    }
}

void drawStartFinishLines() {
    // Starting line (checkered)
    for (int i = -10; i < 10; i++) {
        for (int j = 0; j < 2; j++) {
            glPushMatrix();
            glTranslatef(i + 0.5f, 0.01f, -trackLength / 2 + 5.0f + j * 0.5f);
            glColor3f((i + j) % 2 == 0 ? 1.0f : 0.0f, (i + j) % 2 == 0 ? 1.0f : 0.0f, (i + j) % 2 == 0 ? 1.0f : 0.0f);
            glScalef(1.0f, 0.1f, 0.5f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }

    // Finishing line (checkered)
    for (int i = -10; i < 10; i++) {
        for (int j = 0; j < 2; j++) {
            glPushMatrix();
            glTranslatef(i + 0.5f, 0.01f, trackLength / 2 - 5.0f + j * 0.5f);
            glColor3f((i + j) % 2 == 0 ? 1.0f : 0.0f, (i + j) % 2 == 0 ? 1.0f : 0.0f, (i + j) % 2 == 0 ? 1.0f : 0.0f);
            glScalef(1.0f, 0.1f, 0.5f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
}

void drawWinningBanner() {
    glPushMatrix();
    glTranslatef(0.0f, 8.0f, trackLength / 2 - 5.0f);

    // Banner (now changes height instead of overall scale)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glScalef(15.0f, bannerHeight, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Left pole
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-7.5f, -4.0f, 0.0f);
    glScalef(0.2f, 6.0f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right pole
    glPushMatrix();
    glTranslatef(7.5f, -4.0f, 0.0f);
    glScalef(0.2f, 6.0f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawFlags() {
    for (int i = -1; i <= 1; i += 2) {
        glPushMatrix();
        glTranslatef(i * 9.0f, 3.0f, -trackLength / 2 + 5.0f);

        // Rotate the entire flag assembly
        glRotatef(flagRotation, 0.0f, 1.0f, 0.0f);

        // Pole
        glColor3f(0.5f, 0.5f, 0.5f);
        glPushMatrix();
        glScalef(0.1f, 6.0f, 0.1f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Flag
        glColor3f(1.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0.5f, 2.5f, 0.0f);
        glScalef(1.0f, 0.6f, 0.1f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Ball on top
        glColor3f(1.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 3.1f, 0.0f);
        glutSolidSphere(0.2, 10, 10);

        glPopMatrix();
    }
}

void drawBush(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(bushRotation, 0.0f, 1.0f, 0.0f);  // Add rotation around Y-axis

    // Base (cube)
    glColor3f(0.0f, 0.4f, 0.0f);
    glutSolidCube(1.0);

    // Middle (sphere)
    glTranslatef(0.0f, 0.7f, 0.0f);
    glColor3f(0.0f, 0.6f, 0.0f);
    glutSolidSphere(0.6, 10, 10);

    // Top (tetrahedron)
    glTranslatef(0.0f, 0.5f, 0.0f);
    glColor3f(0.0f, 0.8f, 0.0f);
    glutSolidTetrahedron();

    glPopMatrix();
}

void drawFlower(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(flowerScale, flowerScale, flowerScale);

    // Stem
    glColor3f(0.0f, 0.5f, 0.0f);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.05f, 0.5f, 10, 10);
    glPopMatrix();

    // Center of the flower
    glColor3f(1.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidSphere(0.1f, 10, 10);

    // Petals
    glColor3f(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        glRotatef(60.0f * i, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.1f, 0.0f, 0.0f);
        glutSolidSphere(0.1f, 10, 10);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawBenchArea() {
    // Dark brown space on both sides
    glColor3f(0.2f, 0.1f, 0.05f);
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * (trackWidth / 2 + brownAreaWidth / 2), 0.05f, 0.0f);
        glScalef(brownAreaWidth, 0.1f, trackLength);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Bench (only on one side)
    glPushMatrix();
    glTranslatef(trackWidth / 2 + brownAreaWidth / 2, 0.55f, -40.0f);

    // Bench seat (now gray)
    glColor3fv(benchColor);
    glPushMatrix();
    glScalef(3.0f, 0.2f, 1.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Bench legs (unchanged)
    glColor3f(0.4f, 0.2f, 0.1f);
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            glPushMatrix();
            glTranslatef(i * 1.35f, -0.3f, j * 0.45f);
            glScalef(0.15f, 0.6f, 0.15f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }

    // Water bottle (color changing)
    glColor3fv(benchItemsColor);
    glPushMatrix();
    glTranslatef(-1.0f, 0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(0.15, 0.45, 20, 20);
    glPopMatrix();

    // Bottle cap (color changing)
    glPushMatrix();
    glTranslatef(-1.0f, 0.85f, 0.0f);
    glutSolidSphere(0.075, 10, 10);
    glPopMatrix();

    // Towel (color changing)
    glPushMatrix();
    glTranslatef(1.0f, 0.15f, 0.0f);
    glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.2f, 0.075f, 0.9f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Shoes (unchanged color)
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.5f);
    glScalef(0.3f, 0.1f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.1f, -0.5f);
    glScalef(0.3f, 0.1f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void updateWallColor() {
    wallColorTimer += 0.01f;
    if (wallColorTimer >= 1.0f) {
        wallColorTimer = 0.0f;
        wallColor[0] = 0.5f + (rand() % 30) / 100.0f;
        wallColor[1] = 0.5f + (rand() % 30) / 100.0f;
        wallColor[2] = 0.5f + (rand() % 30) / 100.0f;
    }
}

void updatePlayerPosition() {
    if (!gameOver) {  // Only update position if the game is not over
        if (isJumping) {
            playerY += jumpVelocity;
            jumpVelocity -= 0.02f;  // Gravity

            // Add forward movement during jump
            playerX += jumpForwardVelocity * sin(playerRotation * M_PI / 180.0f);
            playerZ += jumpForwardVelocity * cos(playerRotation * M_PI / 180.0f);
            jumpForwardVelocity *= 0.98f;  // Gradually reduce forward velocity

            if (playerY <= 2.1f) {  // Ground level
                playerY = 2.1f;
                isJumping = false;
                jumpForwardVelocity = 0.0f;

                // Check if player successfully jumped over a hurdle
                float hurdleZ = -35.0f;
                while (hurdleZ <= 35.0f) {
                    if (abs(playerZ - hurdleZ) < 1.0f) {
                        break;
                    }
                    hurdleZ += 10.0f;
                }
            }
            updateCameraPosition();
        }
        else if (isMovingForward) {
            // Continue moving forward after landing
            playerX += 0.1f * sin(playerRotation * M_PI / 180.0f);
            playerZ += 0.1f * cos(playerRotation * M_PI / 180.0f);
        }

        // Add boundary checks
        if (playerX < -trackWidth / 2 + 1.0f) playerX = -trackWidth / 2 + 1.0f; 
        if (playerX > trackWidth / 2 - 1.0f) playerX = trackWidth / 2 - 1.0f;
        if (playerZ < -trackLength / 2 + 1.0f) playerZ = -trackLength / 2 + 1.0f;
        if (playerZ > trackLength / 2 - 1.0f) playerZ = trackLength / 2 - 1.0f;

        checkHurdleCollision();
        checkFinishLine();
    }
}

void checkHurdleCollision() {
    // Simplified collision detection
    float hurdleZ = -35.0f; // Starting Z position of hurdles
    while (hurdleZ <= 35.0f) {
        if (abs(playerZ - hurdleZ) < 0.5f && playerY < 3.0f) {
            playerZ = hurdleZ - 0.5f;
            playCollision();
            break;
        }
        hurdleZ += 10.0f; // Move to next hurdle
    }
}

void checkFinishLine() {
    if (playerZ >= trackLength / 2 - 5.0f) {
        gameOver = true;
        playerWon = true;
        playWinSound();
    }
}

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw timer
    glRasterPos2i(10, 40);
    std::string timeText = "Time: " + std::to_string(static_cast<int>(gameTime));
    for (char c : timeText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void playSoundEffect(const char* soundFile) {

    // Convert const char* to LPCWSTR
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, soundFile, -1, NULL, 0);
    wchar_t* wideSoundFile = new wchar_t[wideCharLength];
    MultiByteToWideChar(CP_UTF8, 0, soundFile, -1, wideSoundFile, wideCharLength);

    if (PlaySound(wideSoundFile, NULL, SND_ASYNC | SND_FILENAME)){
        isSoundEffectPlaying = true;
        /*Sleep(2000);*/
        lastPlayTime = GetTickCount64();
    }
    else {
        // Handle error
        DWORD error = GetLastError();
        // You might want to log this error or display it to the user
    }

    delete[] wideSoundFile;
}

void playFlagRotateSound() {
    playSoundEffect("sounds/flag_rotate.wav");
}

void playWinSound() {
    playSoundEffect("sounds/win.wav");
}

void playLoseSound() {
    playSoundEffect("sounds/lose.wav");
}

void playJumpSound() {
    playSoundEffect("sounds/jump.wav");
}

void playCheerSound() {
    playSoundEffect("sounds/cheer.wav");
}

void playBoundarySound() {
    playSoundEffect("sounds/boundary.wav");
}

void playFlowerGrowSound() {
    playSoundEffect("sounds/flower_grow.wav");
}

void playCollision() {
    playSoundEffect("sounds/collision.wav");
}

void updateAudio() {
    if (isSoundEffectPlaying) {
        DWORD currentTime = GetTickCount64();
        if (currentTime - lastPlayTime > 2000) {
            isSoundEffectPlaying = false;
        }
    }
}

void audioManager() {
    updateAudio();
}