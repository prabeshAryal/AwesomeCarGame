#include "../include/Level.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

const float MIN_SCROLL_SPEED = 0.3f;
const float MAX_SCROLL_SPEED = 2.0f;
const float LEVEL_1_SPEED = 0.2f; // Very slow start
const float LEVEL_2_SPEED = 0.8f; // Medium
const float LEVEL_3_SPEED = 1.6f; // Old level 1 speed (fastest)
const float SPEED_INCREASE_PER_SEC = 0.04f; // How fast the speed ramps up (tweak as desired)

// Add to the top of the file after includes
const char* HIGH_SCORE_FILE = "highscore.txt";

// Add at the top with other constants
const int INITIAL_SCORE = 100;
const int LEVEL_UP_MULTIPLIER = 2; // Double score for next level
const float SCORE_PER_SECOND = 1.0f; // 1 point per second
const float SPEED_SCORE_MULTIPLIER = 0.5f; // Additional points based on speed

Level::Level() : roadWidth(4.5f), roadLength(20.0f), scrollSpeed(LEVEL_1_SPEED),
                score(INITIAL_SCORE), level(1), levelTimer(0.0f),
                obstacleSpawnTimer(0.0f), enemySpawnTimer(0.0f),
                roadOffset(0.0f), roadTextureId(0), scoreTimer(0.0f),
                highScore(0) {
    srand(static_cast<unsigned int>(time(nullptr)));
    loadHighScore();
}

Level::~Level() {
    // Clean up the road texture
    if (roadTextureId != 0) {
        glDeleteTextures(1, &roadTextureId);
    }
}

void Level::loadHighScore() {
    std::ifstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void Level::saveHighScore() {
    std::ofstream file(HIGH_SCORE_FILE);
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void Level::createRoadTexture() {
    const int textureWidth = ROAD_TEXTURE_SIZE;
    const int textureHeight = ROAD_TEXTURE_SIZE;
    unsigned char* textureData = new unsigned char[textureWidth * textureHeight * 3];

    // Calculate where the yellow lines should be in the texture - matching drawLaneLines
    float emergencyMarginRatio = 0.3f / roadWidth; // Match the larger margin in drawLaneLines
    // Place yellow lines slightly inward from the edges to ensure visibility
    int yellowLinePosition = static_cast<int>(textureWidth * 0.2f); // 10% from edge
    
    // Create road texture pattern with much more detailed asphalt
    for (int y = 0; y < textureHeight; y++) {
        for (int x = 0; x < textureWidth; x++) {
            int index = (y * textureWidth + x) * 3;

            // Only draw asphalt and shine, no lines
            unsigned char gray = 60 + (rand() % 20); // More consistent base color
            if (((x + y) % 64 < 3) && (rand() % 10 == 0)) {
                textureData[index] = 200;     // R
                textureData[index + 1] = 200; // G
                textureData[index + 2] = 200; // B
            } else {
                textureData[index] = gray;     // R
                textureData[index + 1] = gray; // G
                textureData[index + 2] = gray; // B
            }
        }
    }

    // Generate texture
    glGenTextures(1, &roadTextureId);
    glBindTexture(GL_TEXTURE_2D, roadTextureId);

    // Set texture parameters for better quality
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    
    // Generate mipmaps for better quality at different distances
    glGenerateMipmap(GL_TEXTURE_2D);

    delete[] textureData;
}

void Level::init() {
    // Create the procedural road texture
    createRoadTexture();
    
    // Load textures for obstacles
    barrierTexture.loadFromFile("textures/barricade.png");
    coneTexture.loadFromFile("textures/cone.png");
    rockTexture.loadFromFile("textures/rock.png");
    treeTexture.loadFromFile("textures/tree.png");
    enemyCarTexture.loadFromFile("textures/enemycar.png");
}

void Level::update(float deltaTime, Car& playerCar) {
    levelTimer += deltaTime;
    obstacleSpawnTimer += deltaTime;
    enemySpawnTimer += deltaTime;

    // Progressive speed increase
    if (scrollSpeed < MAX_SCROLL_SPEED) {
        float speedIncrease = SPEED_INCREASE_PER_SEC * deltaTime;
        if (scrollSpeed > 1.0f) {
            speedIncrease *= 0.5f;
        }
        scrollSpeed += speedIncrease;
        if (scrollSpeed > MAX_SCROLL_SPEED) scrollSpeed = MAX_SCROLL_SPEED;
    }

    // Update road offset for scrolling
    roadOffset += scrollSpeed * deltaTime;
    if (roadOffset > 1.0f) {
        roadOffset -= 1.0f;
    }

    // Update obstacles with current speed
    for (auto& obstacle : obstacles) {
        obstacle.y -= scrollSpeed * deltaTime;
        if (obstacle.y < -roadLength) {
            obstacle.isActive = false;
        }
    }

    // Update enemy cars
    for (auto& enemy : enemyCars) {
        if (enemy.isActive()) {
            enemy.setY(enemy.getY() - enemy.getSpeed() * deltaTime);
            if (enemy.getY() < -roadLength) {
                enemy.setActive(false);
            }
        }
    }

    // Spawn new obstacles with speed-based timing
    float baseObstacleInterval = 3.0f;
    float obstacleSpawnInterval = baseObstacleInterval / scrollSpeed;
    if (obstacleSpawnTimer >= obstacleSpawnInterval) {
        spawnObstacle();
        obstacleSpawnTimer = 0.0f;
    }

    // Spawn new enemy cars with speed-based timing
    float baseEnemyInterval = 6.0f;
    float enemySpawnInterval = baseEnemyInterval / scrollSpeed;
    if (enemySpawnTimer >= enemySpawnInterval) {
        spawnEnemyCar();
        enemySpawnTimer = 0.0f;
    }

    // Update score based on time and speed
    updateTimerScore(deltaTime);
    
    // Update level when score doubles
    int nextLevelScore = INITIAL_SCORE * static_cast<int>(std::pow(LEVEL_UP_MULTIPLIER, level));
    if (score >= nextLevelScore) {
        level++;
    }
    
    // Check collisions
    checkCollisions(playerCar);

    // Update high score if needed
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
}

void Level::updateTimerScore(float deltaTime) {
    scoreTimer += deltaTime;
    if (scoreTimer >= 1.0f) { // Update score every second
        // Base score increases with time
        int baseScore = static_cast<int>(SCORE_PER_SECOND);
        // Additional points based on speed
        int speedBonus = static_cast<int>(scrollSpeed * SPEED_SCORE_MULTIPLIER);
        score += baseScore + speedBonus;
        scoreTimer = 0.0f;
    }
}

void Level::render() {
    drawRoad();
    drawObstacles();

    // Render enemy cars
    for (auto& enemy : enemyCars) {
        if (enemy.isActive()) {
            enemy.render();
        }
    }
}

void Level::drawRoad() {
    // Draw the base road
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roadTextureId);

    glColor3f(1.0f, 1.0f, 1.0f);  // Reset color to white for proper texture rendering
    glBegin(GL_QUADS);
    // Bottom-left
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-roadWidth, -roadLength, -0.1f);
    // Bottom-right
    glTexCoord2f(1.0f, 0.0f);  // Changed from 3.0f to 1.0f to use the texture once
    glVertex3f(roadWidth, -roadLength, -0.1f);
    // Top-right
    glTexCoord2f(1.0f, roadOffset * 5.0f);  // Changed from 3.0f to 1.0f
    glVertex3f(roadWidth, roadLength, -0.1f);
    // Top-left
    glTexCoord2f(0.0f, roadOffset * 5.0f);
    glVertex3f(-roadWidth, roadLength, -0.1f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // Draw lane lines
    drawLaneLines();
}

void Level::drawLaneLines() {
    // Add a small emergency margin beyond the yellow lines (like real roads)
    float emergencyMargin = 0.3f; // Increased margin
    float yellowLinePosition = roadWidth * 0.58f; // Changed from 0.5f to 0.6f to push lines outward
    
    // Draw white dashed lines in the middle of the road
    glColor3f(1.0f, 1.0f, 1.0f); // Pure white
    glLineWidth(6.0f); // Increased from 5.0f - Thicker white lines
    glBegin(GL_LINES);
    
    // Draw white dashed center line
    float dashLength = 0.5f;  // Increased from 0.3f - Longer dashes
    float gapLength = 0.5f;   // Increased from 0.4f - Consistent spacing
    float offset = fmod(-roadOffset * 5.0f, dashLength + gapLength); // Use same speed and direction as road texture
    
    for (float y = -roadLength; y < roadLength; ) {
        float yStart = y + offset;
        float yEnd = yStart + dashLength;
        if (yStart < roadLength) {
            glVertex3f(0.0f, yStart, -0.05f);
            glVertex3f(0.0f, std::min(yEnd, roadLength), -0.05f);
        }
        y += dashLength + gapLength;
    }
    
    glEnd();
    
    // Draw yellow border lines as single, continuous scrolling lines at the road edges
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(8.0f); // Increased from 4.0f to make lines thicker
    glBegin(GL_LINES);
    float yellowOffset = fmod(-roadOffset * 5.0f, dashLength + gapLength);
    // Left border
    glVertex3f(-yellowLinePosition, -roadLength + yellowOffset, 0.0f);
    glVertex3f(-yellowLinePosition, roadLength + yellowOffset, 0.0f);
    // Right border
    glVertex3f(yellowLinePosition, -roadLength + yellowOffset, 0.0f);
    glVertex3f(yellowLinePosition, roadLength + yellowOffset, 0.0f);
    glEnd();
    
    glLineWidth(1.0f); // Reset
}


void Level::drawObstacles() {
    for (const auto& obstacle : obstacles) {
        if (!obstacle.isActive) continue;

        glEnable(GL_TEXTURE_2D);
        switch (obstacle.type) {
            case ObstacleType::BARRIER:
                barrierTexture.bind();
                break;
            case ObstacleType::CONE:
                coneTexture.bind();
                break;
            case ObstacleType::ROCK:
                rockTexture.bind();
                break;
            case ObstacleType::TREE:
                treeTexture.bind();
                break;
            default:
                break;
        }

        // Since all textures are 256x256, we can use a scale of 1.0f
        float scale = 1.0f;

        // Draw obstacle with texture coordinates to correctly orient the texture
        glBegin(GL_QUADS);
        // Bottom-left
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(obstacle.x - obstacle.width/2.0f * scale, obstacle.y - obstacle.height/2.0f * scale, 0.0f);
        // Bottom-right
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(obstacle.x + obstacle.width/2.0f * scale, obstacle.y - obstacle.height/2.0f * scale, 0.0f);
        // Top-right
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(obstacle.x + obstacle.width/2.0f * scale, obstacle.y + obstacle.height/2.0f * scale, 0.0f);
        // Top-left
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(obstacle.x - obstacle.width/2.0f * scale, obstacle.y + obstacle.height/2.0f * scale, 0.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
}

void Level::spawnObstacle() {
    Obstacle obstacle;
    
    // Use the same emergency margin as in drawLaneLines
    float emergencyMargin = 0.3f; // MATCH THE VALUE in drawLaneLines()
    float yellowLinePosition = roadWidth - emergencyMargin;
    
    // Use the same boundary calculations as the car
    float safetyMargin = 0.1f;
    float obstacleWidth = 0.3f; // Use the largest obstacle width
    
    // Calculate spawn boundaries to match car movement bounds
    float leftBound = -yellowLinePosition + obstacleWidth/2.0f + safetyMargin + 1.5f;
    float rightBound = yellowLinePosition - obstacleWidth/2.0f - safetyMargin - 1.5f;
    
    obstacle.x = leftBound + static_cast<float>(rand()) / RAND_MAX * (rightBound - leftBound);
    obstacle.y = roadLength;
    obstacle.isActive = true;

    // Randomly select obstacle type and set dimensions
    int type = rand() % 4;
    switch (type) {
        case 0:
            obstacle.type = ObstacleType::BARRIER;
            obstacle.width = 0.3f;
            obstacle.height = 0.3f;
            break;
        case 1:
            obstacle.type = ObstacleType::CONE;
            obstacle.width = 0.2f;
            obstacle.height = 0.2f;
            break;
        case 2:
            obstacle.type = ObstacleType::ROCK;
            obstacle.width = 0.25f;
            obstacle.height = 0.25f;
            break;
        case 3:
            obstacle.type = ObstacleType::TREE;
            obstacle.width = 0.25f;
            obstacle.height = 0.25f;
            break;
    }

    obstacles.push_back(obstacle);
}

void Level::spawnEnemyCar() {
    Car enemy;
    
    // Use the same emergency margin as in drawLaneLines
    float emergencyMargin = 0.3f; // MATCH THE VALUE in drawLaneLines()
    float yellowLinePosition = roadWidth - emergencyMargin;
    
    // Use the same boundary calculations as the car
    float safetyMargin = 0.1f;
    float enemyCarWidth = 0.2f;
    
    // Calculate spawn boundaries to match car movement bounds
    float leftBound = -yellowLinePosition + enemyCarWidth/2.0f + safetyMargin + 1.5f;
    float rightBound = yellowLinePosition - enemyCarWidth/2.0f - safetyMargin - 1.5f;
    
    enemy.setX(leftBound + static_cast<float>(rand()) / RAND_MAX * (rightBound - leftBound));
    enemy.setY(roadLength); // Start at the top of the road
    
    // Set the enemy car to move downward at a speed relative to current road speed
    // Make enemy cars slightly faster than the road to increase challenge
    float enemySpeed = scrollSpeed * 1.5f; // 50% faster than road speed
    enemy.setSpeed(enemySpeed);
    enemy.setActive(true);
    
    // Initialize the enemy car with the enemy car texture
    enemy.initAsEnemy();
    
    // Add to enemy cars vector
    enemyCars.push_back(enemy);
}

void Level::checkCollisions(Car& playerCar) {
    // Check obstacle collisions
    for (auto& obstacle : obstacles) {
        if (!obstacle.isActive) continue;

        if (playerCar.checkCollision(obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
            obstacle.isActive = false;
            playerCar.takeDamage(25.0f);
            if (playerCar.getHealth() <= 0.0f) {
                // Game over
                return;
            }
        }
    }

    // Check enemy car collisions
    for (auto& enemy : enemyCars) {
        if (!enemy.isActive()) continue;

        if (playerCar.checkCollision(enemy.getX(), enemy.getY(), enemy.getWidth(), enemy.getHeight())) {
            enemy.setActive(false);
            playerCar.takeDamage(playerCar.getHealth()); // Set health to 0 (game over)
            return;
        }
    }

    // Check projectile collisions with obstacles
    for (auto& obstacle : obstacles) {
        if (!obstacle.isActive) continue;

        // Get projectile collision directly from player car
        // This will deactivate the projectile if it hits
        if (playerCar.checkProjectileCollision(obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
            // If collision detected, deactivate obstacle and add score
            obstacle.isActive = false;
            score += OBSTACLE_POINTS; // Add points for destroying an obstacle
            std::cout << "Obstacle hit! Score: " << score << std::endl; // Debug output
        }
    }

    // Check projectile collisions with enemy cars
    for (auto& enemy : enemyCars) {
        if (!enemy.isActive()) continue;

        if (playerCar.checkProjectileCollision(enemy.getX(), enemy.getY(), enemy.getWidth(), enemy.getHeight())) {
            enemy.setActive(false);
            score += 50; // Add 50 points for destroying an enemy car
            std::cout << "Enemy car hit! Score: " << score << std::endl; // Debug output
        }
    }
}

void Level::updateLevel() {
    // Only update level number for display/logic, not for speed
    int newLevel = 1; // Default to level 1
    if (score >= LEVEL_3_SCORE) {
        newLevel = 3;
    } else if (score >= LEVEL_2_SCORE) {
        newLevel = 2;
    } else if (score >= LEVEL_1_SCORE) {
        newLevel = 1;
    }
    level = newLevel;
}

bool Level::isComplete() const {
    return false; // Game continues indefinitely
}

int Level::getScore() const {
    return score;
}

int Level::getLevel() const {
    return level;
}

int Level::getHighScore() const {
    return highScore;
}

void Level::reset() {
    obstacles.clear();
    enemyCars.clear();
    score = INITIAL_SCORE; // Reset to initial score
    level = 1;
    scrollSpeed = LEVEL_1_SPEED;
    levelTimer = 0.0f;
    obstacleSpawnTimer = 0.0f;
    enemySpawnTimer = 0.0f;
    roadOffset = 0.0f;
    scoreTimer = 0.0f;
} 