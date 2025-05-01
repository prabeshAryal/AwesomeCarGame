#include <iostream>
#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <vector>
#include <string>
#include <ctime>
#include "include/Car.h"
#include "include/Level.h"
#include "include/Texture.h"

// Game constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float ASPECT_RATIO = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;

// Game state
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Global game objects
GameState currentState = GameState::MENU;
Car playerCar;
Level currentLevel;
float deltaTime = 0.0f;
float lastTime = 0.0f;
int menuIndex = 0;
const int MENU_ITEMS = 2;
const char* menuOptions[MENU_ITEMS] = { "New Game", "Exit" };

// Function declarations
void init();
void startNewGame();
void renderText(float x, float y, const char* text);
void renderMenu();
void renderGameOver();
void renderPause();
void renderGame();
void display();
void update(int value);
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void reshape(int w, int h);
void specialKeys(int key, int x, int y);
void specialKeysUp(int key, int x, int y);

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Need for Speed Infinity");
    
    // Center the window on screen but don't maximize
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutPositionWindow((screenWidth - WINDOW_WIDTH) / 2, (screenHeight - WINDOW_HEIGHT) / 2);
    
    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // Initialize game
    init();

    // Set up callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0); // ~60 FPS

    // Start main loop
    glutMainLoop();

    return 0;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    playerCar.init();
    currentLevel.init();
    currentState = GameState::PLAYING;
}

void startNewGame() {
    playerCar.~Car();
    new (&playerCar) Car();
    playerCar.init();

    currentLevel.~Level();
    new (&currentLevel) Level();
    currentLevel.init();

    currentState = GameState::PLAYING;
}

void renderText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void renderMenu() {
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(-0.35f, 0.5f, "NEED FOR SPEED INFINITY");
    for (int i = 0; i < MENU_ITEMS; ++i) {
        if (i == menuIndex) {
            glColor3f(1.0f, 1.0f, 0.0f); // Highlighted
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        renderText(-0.15f, 0.2f - i * 0.15f, menuOptions[i]);
    }
    glColor3f(0.7f, 0.7f, 1.0f);
    renderText(-0.45f, -0.2f, "Controls: Arrow Keys to move, Space to fire");
    renderText(-0.45f, -0.3f, "Menu: Up/Down to select, Enter to confirm");
}

void renderGameOver() {
    glColor3f(1.0f, 0.0f, 0.0f);
    renderText(-0.2f, 0.2f, "GAME OVER");
    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", currentLevel.getScore());
    renderText(-0.1f, 0.0f, scoreText);
    renderText(-0.3f, -0.2f, "Press SPACE for menu");
}

void renderPause() {
    glColor3f(1.0f, 1.0f, 0.0f);
    renderText(-0.1f, 0.0f, "PAUSED");
    renderText(-0.3f, -0.2f, "Press P to resume");
}

void renderGame() {
    // Render level
    currentLevel.render();
    
    // Render player car
    playerCar.render();
    
    // Save current matrix and disable depth test temporarily for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    // Use orthographic projection for HUD
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Disable depth test for HUD elements
    glDisable(GL_DEPTH_TEST);
    
    // Set HUD color
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow text
    
    // Draw HUD text
    char scoreText[32], healthText[32], levelText[32];
    sprintf_s(scoreText, "Score: %d", currentLevel.getScore());
    sprintf_s(healthText, "Health: %.0f", playerCar.getHealth());
    sprintf_s(levelText, "Level: %d", currentLevel.getLevel());
    
    renderText(-0.95f, 0.9f, scoreText);
    renderText(-0.1f, 0.9f, levelText);
    renderText(0.6f, 0.9f, healthText);
    
    // Restore settings
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Get current window dimensions
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float aspectRatio = (float)w / (float)h;
    
    // Set up projection matrix to maintain proper aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Use an orthographic projection with aspect ratio correction
    if (w <= h) {
        gluOrtho2D(-1.0, 1.0, -1.0/aspectRatio, 1.0/aspectRatio);
    } else {
        gluOrtho2D(-1.0*aspectRatio, 1.0*aspectRatio, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Render based on current state
    switch (currentState) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::PLAYING:
            renderGame();
            break;
        case GameState::PAUSED:
            renderGame();
            renderPause();
            break;
        case GameState::GAME_OVER:
            renderGameOver();
            break;
    }
    
    glutSwapBuffers();
}

void update(int value) {
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    if (currentState == GameState::PLAYING) {
        playerCar.update(deltaTime);
        currentLevel.update(deltaTime, playerCar);
        currentLevel.checkCollisions(playerCar);
        if (playerCar.getHealth() <= 0) {
            currentState = GameState::GAME_OVER;
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (currentState == GameState::MENU) {
        if (key == 13) { // Enter
            if (menuIndex == 0) {
                startNewGame();
            } else if (menuIndex == 1) {
                exit(0);
            }
        }
        return;
    }
    switch (key) {
        case 'w':
        case 'W':
            playerCar.moveUp();
            break;
        case 's':
        case 'S':
            playerCar.moveDown();
            break;
        case 'a':
        case 'A':
            playerCar.moveLeft();
            break;
        case 'd':
        case 'D':
            playerCar.moveRight();
            break;
        case ' ':
            if (currentState == GameState::GAME_OVER) {
                currentState = GameState::MENU;
                glutPostRedisplay();
                return;
            } else if (currentState == GameState::PLAYING) {
                playerCar.fire();
            }
            break;
        case 'p':
        case 'P':  // P key to pause/unpause
            if (currentState == GameState::PLAYING) {
                currentState = GameState::PAUSED;
            } else if (currentState == GameState::PAUSED) {
                currentState = GameState::PLAYING;
            }
            break;
        case 27:   // ESC key
            exit(0);
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
            playerCar.stopVertical();
            break;
        case 's':
        case 'S':
            playerCar.stopVertical();
            break;
        case 'a':
        case 'A':
            playerCar.stopHorizontal();
            break;
        case 'd':
        case 'D':
            playerCar.stopHorizontal();
            break;
    }
}

void reshape(int w, int h) {
    // Set the viewport to cover the new window size
    glViewport(0, 0, w, h);
    
    // Calculate aspect ratio
    float aspectRatio = (float)w / (float)h;
    
    // Set up projection matrix to maintain proper aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Use an orthographic projection with aspect ratio correction
    if (w <= h) {
        gluOrtho2D(-1.0, 1.0, -1.0/aspectRatio, 1.0/aspectRatio);
    } else {
        gluOrtho2D(-1.0*aspectRatio, 1.0*aspectRatio, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void specialKeys(int key, int x, int y) {
    if (currentState == GameState::MENU) {
        if (key == GLUT_KEY_UP) {
            menuIndex = (menuIndex - 1 + MENU_ITEMS) % MENU_ITEMS;
        } else if (key == GLUT_KEY_DOWN) {
            menuIndex = (menuIndex + 1) % MENU_ITEMS;
        }
        glutPostRedisplay();
        return;
    }
    switch (key) {
        case GLUT_KEY_UP:
            playerCar.moveUp();
            break;
        case GLUT_KEY_DOWN:
            playerCar.moveDown();
            break;
        case GLUT_KEY_LEFT:
            playerCar.moveLeft();
            break;
        case GLUT_KEY_RIGHT:
            playerCar.moveRight();
            break;
    }
}

void specialKeysUp(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN:
            playerCar.stopVertical();
            break;
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT:
            playerCar.stopHorizontal();
            break;
    }
}