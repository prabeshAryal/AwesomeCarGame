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
    
    // Get screen dimensions
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    
    // Create full screen window
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Need for Speed Infinity");
    glutFullScreen(); // Make window full screen
    
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
    // Title
    glColor3f(1.0f, 0.5f, 0.0f); // Orange color for title
    renderText(-0.3f, 0.6f, "NEED FOR SPEED");
    renderText(-0.2f, 0.5f, "INFINITY");
    
    // Separator line
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.5f, 0.45f);
    glVertex2f(0.5f, 0.45f);
    glEnd();

    // Display high score with golden color
    glColor3f(1.0f, 0.84f, 0.0f); // Gold color
    char highScoreText[32];
    snprintf(highScoreText, sizeof(highScoreText), "Best Score: %d", currentLevel.getHighScore());
    renderText(-0.2f, 0.35f, highScoreText);
    
    // Menu items with better spacing and highlighting
    const float menuStartY = 0.1f;
    const float menuSpacing = 0.15f;
    
    for (int i = 0; i < MENU_ITEMS; ++i) {
        float y = menuStartY - i * menuSpacing;
        
        // Draw selection marker
        if (i == menuIndex) {
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow for selected item
            renderText(-0.25f, y, "> ");
            renderText(0.2f, y, " <");
        } else {
            glColor3f(0.7f, 0.7f, 0.7f); // Grey for unselected items
        }
        
        // Center the menu item text
        float textWidth = strlen(menuOptions[i]) * 0.02f; // Approximate width per character
        float x = -textWidth;
        renderText(x, y, menuOptions[i]);
    }
    
    // Separator line
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.5f, -0.1f);
    glVertex2f(0.5f, -0.1f);
    glEnd();
    
    // Controls info at bottom
    glColor3f(0.5f, 0.5f, 1.0f); // Light blue for controls
    renderText(-0.45f, -0.3f, "Controls:");
    glColor3f(0.7f, 0.7f, 0.7f); // Grey for control text
    renderText(-0.45f, -0.4f, "Arrow Keys - Move    Space - Fire");
    renderText(-0.45f, -0.5f, "P - Pause    ESC - Exit");
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
    
    // Draw HUD text with better positioning and formatting
    char scoreText[32], highScoreText[32], healthText[32], levelText[32];
    sprintf_s(scoreText, "Score: %d", currentLevel.getScore());
    sprintf_s(highScoreText, "Best: %d", currentLevel.getHighScore());
    sprintf_s(healthText, "HP: %.0f%%", playerCar.getHealth());
    sprintf_s(levelText, "Lvl: %d", currentLevel.getLevel());
    
    // Calculate the same boundaries as car movement
    float emergencyMargin = 0.3f;
    float yellowLinePosition = 4.5f - emergencyMargin; // 4.5f is roadWidth
    float safetyMargin = 0.1f;
    float carWidth = 0.2f;
    float leftBound = -yellowLinePosition + carWidth/2.0f + safetyMargin + 1.5f;
    float rightBound = yellowLinePosition - carWidth/2.0f - safetyMargin - 1.5f;
    
    // Convert road bounds to screen coordinates (approximately)
    float screenLeftBound = leftBound / 4.5f;  // Normalize by roadWidth
    float screenRightBound = rightBound / 4.5f;
    
    // Position all elements in one line with even spacing
    float totalWidth = screenRightBound - screenLeftBound;
    float spacing = totalWidth / 4.0f;  // Divide space into 4 sections
    
    // Score (leftmost)
    glColor3f(1.0f, 1.0f, 1.0f);  // White text for score
    renderText(screenLeftBound, 0.9f, scoreText);
    
    // High Score
    glColor3f(1.0f, 0.84f, 0.0f);  // Gold color for high score
    renderText(screenLeftBound + spacing, 0.9f, highScoreText);
    
    // Level
    glColor3f(0.0f, 1.0f, 1.0f);  // Cyan for level
    renderText(screenLeftBound + spacing * 2, 0.9f, levelText);
    
    // Health (rightmost)
    float healthPercent = playerCar.getHealth();
    if (healthPercent > 60.0f) {
        glColor3f(0.0f, 1.0f, 0.0f);  // Green for good health
    } else if (healthPercent > 30.0f) {
        glColor3f(1.0f, 1.0f, 0.0f);  // Yellow for medium health
    } else {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for low health
    }
    renderText(screenLeftBound + spacing * 3, 0.9f, healthText);
    
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
    // Scale the view to maintain the same relative sizes
    float scale = 1.0f;
    if (w > h) {
        scale = (float)h / (float)w;
        gluOrtho2D(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f);
    } else {
        scale = (float)w / (float)h;
        gluOrtho2D(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Apply scaling to maintain consistent sizes
    glScalef(scale, scale, 1.0f);
    
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
    // Scale the view to maintain the same relative sizes
    float scale = 1.0f;
    if (w > h) {
        scale = (float)h / (float)w;
        gluOrtho2D(-1.0f * aspectRatio, 1.0f * aspectRatio, -1.0f, 1.0f);
    } else {
        scale = (float)w / (float)h;
        gluOrtho2D(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio);
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