#include "../include/Level.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

const float MIN_SCROLL_SPEED = 0.8f;
const float MAX_SCROLL_SPEED = 2.0f;
const float LEVEL_1_SPEED = 0.4f; // Very slow start
const float LEVEL_2_SPEED = 1.2f; // Medium
const float LEVEL_3_SPEED = 2.0f; // Old level 1 speed (fastest)
const float SPEED_INCREASE_PER_SEC = 0.08f; // How fast the speed ramps up (tweak as desired)

Level::Level() : roadWidth(4.5f), roadLength(20.0f), scrollSpeed(LEVEL_1_SPEED),
                score(0), level(1), levelTimer(0.0f),
                obstacleSpawnTimer(0.0f), enemySpawnTimer(0.0f),
                roadOffset(0.0f), roadTextureId(0), scoreTimer(0.0f) {
    srand(static_cast<unsigned int>(time(nullptr)));
}

Level::~Level() {
    // Clean up the road texture
    if (roadTextureId != 0) {
        glDeleteTextures(1, &roadTextureId);
    }
}

void Level::createRoadTexture() {
    const int textureWidth = ROAD_TEXTURE_SIZE;
    const int textureHeight = ROAD_TEXTURE_SIZE;
    unsigned char* textureData = new unsigned char[textureWidth * textureHeight * 3];

    // Calculate where the yellow lines should be in the texture - matching drawLaneLines
    float emergencyMarginRatio = 0.3f / roadWidth; // Match the larger margin in drawLaneLines
    int yellowLinePosition = static_cast<int>((1.0f - emergencyMarginRatio) * textureWidth / 2);
    
    // Create road texture pattern with SINGLE set of yellow lines at edges
    for (int y = 0; y < textureHeight; y++) {
        for (int x = 0; x < textureWidth; x++) {
            int index = (y * textureWidth + x) * 3;

            // Dark gray asphalt base
            unsigned char gray = 50 + (rand() % 15); // Darker with less noise

            // ONLY ONE set of yellow lines at each edge - make it 3 pixels wide
            if (x >= yellowLinePosition - 1 && x <= yellowLinePosition + 1) {
                // Left side yellow line (3 pixels wide)
                textureData[index] = 240;     // R
                textureData[index + 1] = 240; // G
                textureData[index + 2] = 0;   // B
            }
            else if (x >= textureWidth - yellowLinePosition - 1 && x <= textureWidth - yellowLinePosition + 1) {
                // Right side yellow line (3 pixels wide)
                textureData[index] = 240;     // R
                textureData[index + 1] = 240; // G
                textureData[index + 2] = 0;   // B
            }
            // White center line ONLY - NO yellow in middle
            else if (x == textureWidth / 2 && y % 24 < 12) {
                textureData[index] = 255;     // R
                textureData[index + 1] = 255; // G
                textureData[index + 2] = 255; // B
            }
            // Road shine effect 
            else if (((x + y) % 64 < 3) && (rand() % 10 == 0)) {
                textureData[index] = 170;     // R
                textureData[index + 1] = 170; // G
                textureData[index + 2] = 170; // B
            }
            // Base asphalt color
            else {
                textureData[index] = gray;     // R
                textureData[index + 1] = gray; // G
                textureData[index + 2] = gray; // B
            }
        }
    }

    // Generate texture
    glGenTextures(1, &roadTextureId);
    glBindTexture(GL_TEXTURE_2D, roadTextureId);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

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

    // Speed logic: slow start, ramp up to next level, then jump to next speed
    if (score < LEVEL_2_SCORE) {
        if (scrollSpeed < LEVEL_2_SPEED) {
            scrollSpeed += SPEED_INCREASE_PER_SEC * deltaTime;
            if (scrollSpeed > LEVEL_2_SPEED) scrollSpeed = LEVEL_2_SPEED;
        }
    } else if (score < LEVEL_3_SCORE) {
        scrollSpeed = LEVEL_2_SPEED;
    } else {
        scrollSpeed = LEVEL_3_SPEED;
    }

    // Update road offset for scrolling
    roadOffset += scrollSpeed * deltaTime;
    if (roadOffset > 1.0f) {
        roadOffset -= 1.0f;
    }

    // Update obstacles
    for (auto& obstacle : obstacles) {
        obstacle.y -= scrollSpeed * deltaTime;
        if (obstacle.y < -roadLength) {
            obstacle.isActive = false;
        }
    }

    // Update enemy cars - move by their own speed
    for (auto& enemy : enemyCars) {
        if (enemy.isActive()) {
            enemy.setY(enemy.getY() - enemy.getSpeed() * deltaTime);
            if (rand() % 100 == 0) {
                std::cout << "Enemy car at position (" << enemy.getX() << ", " << enemy.getY() << ")" << std::endl;
            }
            if (enemy.getY() < -roadLength) {
                enemy.setActive(false);
                std::cout << "Enemy car went off-screen" << std::endl;
            }
        }
    }

    // Spawn new obstacles
    if (obstacleSpawnTimer >= OBSTACLE_SPAWN_INTERVAL) {
        spawnObstacle();
        obstacleSpawnTimer = 0.0f;
    }

    // Spawn new enemy cars
    if (enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
        spawnEnemyCar();
        enemySpawnTimer = 0.0f;
    }

    // Update score based on time
    updateTimerScore(deltaTime);
    
    // Check collisions
    checkCollisions(playerCar);

    // Update level based on score
    updateLevel();
}

void Level::updateTimerScore(float deltaTime) {
    scoreTimer += deltaTime;
    if (scoreTimer >= TIME_SCORE_INTERVAL) {
        score += TIME_SCORE_POINTS;
        scoreTimer -= TIME_SCORE_INTERVAL;
    }
}

void Level::render() {
    drawRoad();
    drawObstacles();

    // Render enemy cars with their texture
    for (auto& enemy : enemyCars) {
        if (enemy.isActive()) {
            // Reset color to white for proper texture rendering
            glColor3f(1.0f, 1.0f, 1.0f);
            
            // Make sure blending is enabled for transparent textures
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Let the enemy car handle its own rendering
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
    glTexCoord2f(3.0f, 0.0f);
    glVertex3f(roadWidth, -roadLength, -0.1f);
    // Top-right
    glTexCoord2f(3.0f, roadOffset * 5.0f);
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
    float yellowLinePosition = roadWidth - emergencyMargin;
    
    // Draw ONLY ONE set of yellow border lines near the edge with greater width
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(8.0f); // Make the yellow lines much thicker and more visible
    glBegin(GL_LINES);
    // Left border - ONLY ONE LINE
    glVertex3f(-yellowLinePosition, -roadLength, 0.02f);
    glVertex3f(-yellowLinePosition, roadLength, 0.02f);
    // Right border - ONLY ONE LINE
    glVertex3f(yellowLinePosition, -roadLength, 0.02f);
    glVertex3f(yellowLinePosition, roadLength, 0.02f);
    glEnd();

    // Draw white lines ONLY in the center (NO yellow in middle)
    glColor3f(1.0f, 1.0f, 1.0f); // Pure white
    glLineWidth(5.0f); // White lines in center
    glBegin(GL_LINES);
    float dashLength = 0.3f;
    float gapLength = 0.4f;
    float offset = fmod(roadOffset * 2.0f, dashLength + gapLength);
    
    // Draw ONLY white center line
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
    float emergencyMargin = 0.2f; 
    float yellowLinePosition = roadWidth - emergencyMargin;
    
    // Ensure obstacles are well within the yellow lines
    float safetyMargin = 0.1f;
    float obstacleWidth = 0.3f; // Use the largest obstacle width
    
    // Calculate spawn boundaries to keep obstacles fully within yellow lines
    float minX = -yellowLinePosition + obstacleWidth/2.0f + safetyMargin;
    float maxX = yellowLinePosition - obstacleWidth/2.0f - safetyMargin;
    
    obstacle.x = minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX);
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
    float emergencyMargin = 0.2f; 
    float yellowLinePosition = roadWidth - emergencyMargin;
    
    // Ensure enemy cars are well within the yellow lines
    float safetyMargin = 0.1f;
    float enemyCarWidth = 0.2f;
    
    // Calculate spawn boundaries to keep enemy cars fully within yellow lines
    float minX = -yellowLinePosition + enemyCarWidth/2.0f + safetyMargin;
    float maxX = yellowLinePosition - enemyCarWidth/2.0f - safetyMargin;
    
    enemy.setX(minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX));
    enemy.setY(roadLength); // Start at the top of the road
    
    // Set the enemy car to move downward at the same speed as the road
    float enemySpeed = scrollSpeed*2;
    enemy.setSpeed(enemySpeed);
    enemy.setActive(true);
    
    // Initialize the enemy car with the enemy car texture
    enemy.initAsEnemy();
    
    std::cout << "Spawned enemy car at position (" << enemy.getX() << ", " << enemy.getY() 
              << ") with speed " << enemySpeed << std::endl;
    
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

void Level::reset() {
    obstacles.clear();
    enemyCars.clear();
    score = 0;
    level = 1;
    scrollSpeed = LEVEL_1_SPEED;
    levelTimer = 0.0f;
    obstacleSpawnTimer = 0.0f;
    enemySpawnTimer = 0.0f;
    roadOffset = 0.0f;
    scoreTimer = 0.0f;
} 