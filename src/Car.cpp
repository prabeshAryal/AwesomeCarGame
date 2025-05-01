#include "../include/Car.h"
#include <cmath>
#include <iostream>

// Static members for shared enemy car texture
Texture Car::enemyCarTexture;
bool Car::enemyCarTextureLoaded = false;

Car::Car() : x(0.0f), y(-1.6f), width(0.3f), height(0.3f),  // Position car at bottom
             speed(3.0f), health(100.0f), score(0),
             active(true), fireCooldown(FIRE_COOLDOWN), fireTimer(0.0f),
             horizontalSpeed(0.0f), verticalSpeed(0.0f) {}

Car::~Car() {}

void Car::init() {
    carTexture.loadFromFile("textures/car.png");
}

void Car::ensureEnemyCarTextureLoaded() {
    if (!enemyCarTextureLoaded) {
        if (enemyCarTexture.loadFromFile("textures/enemycar.png")) {
            std::cout << "Loaded shared enemy car texture" << std::endl;
        } else {
            std::cout << "Failed to load shared enemy car texture!" << std::endl;
        }
        enemyCarTextureLoaded = true;
    }
}

void Car::initAsEnemy() {
    ensureEnemyCarTextureLoaded();
    isEnemy = true;
    // For enemy cars, set vertical speed to 0 as we'll control it in the Level class
    verticalSpeed = 0.0f;
    horizontalSpeed = 0.0f;
}

void Car::update(float deltaTime) {
    // Calculate current position after potential movement
    float newX = x + horizontalSpeed * deltaTime;
    float newY = y + verticalSpeed * deltaTime;
    
    // Use the exact same emergency margin as in drawLaneLines
    float emergencyMargin = 0.3f; // MUST match the value in Level::drawLaneLines
    float yellowLinePosition = 4.5f - emergencyMargin;
    
    // Use the exact visual car dimensions
    float carWidth = 0.2f; // The visual width of the car sprite
    float safetyMargin = 0.1f; // Large safety margin to absolutely prevent crossing
    
    // Calculate exact bounds to strictly enforce yellow line boundaries
    float leftBound = -yellowLinePosition + carWidth/2.0f + safetyMargin +1.5f;
    float rightBound = yellowLinePosition - carWidth/2.0f - safetyMargin -1.5f;
    
    // Display boundary and position info when moving
    if (horizontalSpeed != 0.0f) {
        std::cout << "Car update - Current X: " << x << ", New X: " << newX 
                  << ", leftBound: " << leftBound << ", rightBound: " << rightBound << std::endl;
    }
    
    // Apply horizontal movement with strict boundaries
    if (newX < leftBound) {
        x = leftBound; // Clamp to left boundary
        horizontalSpeed = 0.0f; // Stop horizontal movement
    } else if (newX > rightBound) {
        x = rightBound; // Clamp to right boundary
        horizontalSpeed = 0.0f; // Stop horizontal movement
    } else {
        x = newX; // Apply movement normally
    }
    
    // Apply vertical movement
    float carHeight = 0.4f;
    float bottomBound = -1.8f + carHeight/2.0f;
    float topBound = -0.5f - carHeight/2.0f;
    
    if (newY < bottomBound) {
        y = bottomBound;
        verticalSpeed = 0.0f; // Stop vertical movement
    } else if (newY > topBound) {
        y = topBound;
        verticalSpeed = 0.0f; // Stop vertical movement
    } else {
        y = newY;
    }
    
    // Update fire cooldown
    if (fireTimer > 0.0f) {
        fireTimer -= deltaTime;
    }

    // Update projectiles
    for (auto& projectile : projectiles) {
        if (projectile.isActive) {
            projectile.y += PROJECTILE_SPEED * deltaTime;
            
            if (rand() % 100 == 0) {
                std::cout << "Projectile at (" << projectile.x << ", " << projectile.y << ")" << std::endl;
            }
            
            if (projectile.y > 1.0f) {
                projectile.isActive = false;
                std::cout << "Projectile went off-screen" << std::endl;
            }
        }
    }

    // Remove inactive projectiles
    size_t initialSize = projectiles.size();
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const Projectile& p) { return !p.isActive; }),
        projectiles.end()
    );
    
    if (projectiles.size() < initialSize) {
        std::cout << "Removed " << (initialSize - projectiles.size()) << " inactive projectiles" << std::endl;
    }
}

void Car::render() const {
    if (!active) return;

    // Reset color to white before drawing the car with texture
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw car
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use consistent sizing with obstacles but adjust for the 128x256 aspect ratio
    float carWidth = 0.2f;  // Adjusted width 
    float carHeight = 0.4f; // Adjusted height to match 128x256 aspect ratio
    
    // For visualization purposes, make enemy cars slightly smaller than player car
    if (isEnemy) {
        carWidth *= 0.8f;
        carHeight *= 0.8f;
        enemyCarTexture.bind();
    } else {
        carTexture.bind();
    }

    glBegin(GL_QUADS);
    // Bottom-left
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - carWidth/2.0f, y - carHeight/2.0f, 0.0f);
    // Bottom-right
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + carWidth/2.0f, y - carHeight/2.0f, 0.0f);
    // Top-right
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + carWidth/2.0f, y + carHeight/2.0f, 0.0f);
    // Top-left
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - carWidth/2.0f, y + carHeight/2.0f, 0.0f);
    glEnd();

    if (isEnemy) {
        enemyCarTexture.unbind();
    } else {
        carTexture.unbind();
    }
    glDisable(GL_TEXTURE_2D);

    // Render projectiles with more visible size and color
    glColor3f(1.0f, 0.0f, 0.0f); // Bright red for better visibility
    for (const auto& projectile : projectiles) {
        if (projectile.isActive) {
            // Draw a larger, more visible projectile
            glBegin(GL_QUADS);
            glVertex3f(projectile.x - 0.05f, projectile.y - 0.05f, 0.0f); // Larger size
            glVertex3f(projectile.x + 0.05f, projectile.y - 0.05f, 0.0f);
            glVertex3f(projectile.x + 0.05f, projectile.y + 0.05f, 0.0f);
            glVertex3f(projectile.x - 0.05f, projectile.y + 0.05f, 0.0f);
            glEnd();
        }
    }
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
}

void Car::moveLeft() {
    horizontalSpeed = -speed;
}

void Car::moveRight() {
    horizontalSpeed = speed;
}

void Car::moveUp() {
    verticalSpeed = speed;
    std::cout << "Moving up. Current position: (" << x << ", " << y << ")" << std::endl;
}

void Car::moveDown() {
    verticalSpeed = -speed;
    std::cout << "Moving down. Current position: (" << x << ", " << y << ")" << std::endl;
}

void Car::stopHorizontal() {
    horizontalSpeed = 0.0f;
}

void Car::stopVertical() {
    verticalSpeed = 0.0f;
}

void Car::fire() {
    if (fireTimer <= 0.0f) {
        // Calculate the car height for projectile positioning
        float carHeight = 0.4f;
        
        Projectile projectile;
        projectile.x = x;
        projectile.y = y + carHeight/2.0f + 0.05f; // Use the new car height
        projectile.speed = PROJECTILE_SPEED;
        projectile.isActive = true;
        projectiles.push_back(projectile);
        fireTimer = fireCooldown;
        
        std::cout << "Fired projectile at position (" << projectile.x << ", " << projectile.y << ")" << std::endl;
    }
}

bool Car::checkCollision(float x, float y, float width, float height) const {
    // Calculate car dimensions based on whether it's the player or enemy car
    float carWidth = 0.2f;
    float carHeight = 0.4f;
    
    // Enemy cars are smaller (same logic as in render)
    if (verticalSpeed == 0.0f && horizontalSpeed == 0.0f) {
        carWidth *= 0.8f;
        carHeight *= 0.8f;
    }
    
    // Check for intersection using the calculated dimensions
    return (this->x + carWidth/2.0f > x - width/2.0f &&
            this->x - carWidth/2.0f < x + width/2.0f &&
            this->y + carHeight/2.0f > y - height/2.0f &&
            this->y - carHeight/2.0f < y + height/2.0f);
}

bool Car::checkProjectileCollision(float x, float y, float width, float height) const {
    // Projectile size should match the rendered size (0.05f)
    const float PROJECTILE_SIZE = 0.05f;
    
    for (auto& projectile : const_cast<std::vector<Projectile>&>(projectiles)) {
        if (projectile.isActive &&
            projectile.x + PROJECTILE_SIZE > x - width/2.0f &&
            projectile.x - PROJECTILE_SIZE < x + width/2.0f &&
            projectile.y + PROJECTILE_SIZE > y - height/2.0f &&
            projectile.y - PROJECTILE_SIZE < y + height/2.0f) {
            // Deactivate the projectile when it hits something
            projectile.isActive = false;
            std::cout << "Projectile hit at position (" << projectile.x << ", " << projectile.y << ")" << std::endl;
            return true;
        }
    }
    return false;
}

void Car::takeDamage(float amount) {
    health -= amount;
    if (health <= 0.0f) {
        active = false;
    }
}

void Car::addScore(int points) {
    score += points;
}

float Car::getHealth() const {
    return health;
}

int Car::getScore() const {
    return score;
}

float Car::getX() const {
    return x;
}

float Car::getY() const {
    return y;
}

float Car::getWidth() const {
    return width;
}

float Car::getHeight() const {
    return height;
}

float Car::getSpeed() const {
    return speed;
}

bool Car::isActive() const {
    return active;
}

void Car::setX(float x) {
    this->x = x;
}

void Car::setY(float y) {
    this->y = y;
}

void Car::setSpeed(float speed) {
    this->speed = speed;
}

void Car::setActive(bool active) {
    this->active = active;
}

const std::vector<Car::Projectile>& Car::getProjectiles() const {
    return projectiles;
}
