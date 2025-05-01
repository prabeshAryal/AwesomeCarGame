#include "../include/Car.h"
#include <cmath>
#include <iostream>

// Static members for shared enemy car texture
Texture Car::enemyCarTexture;
bool Car::enemyCarTextureLoaded = false;

Car::Car() : x(0.0f), y(-0.5f), width(0.3f), height(0.3f),  // Position car higher up the screen
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
    // Update position based on speed
    x += horizontalSpeed * deltaTime;
    y += verticalSpeed * deltaTime;

    // Keep car within bounds, but allow for more vertical movement space
    // Prevent crossing yellow border lines (matching drawLaneLines)
    float border = 0.02f;
    float margin = 0.01f;
    float carRenderWidth = 0.2f; // Use the actual rendered width
    float leftBound = -1.0f + border + carRenderWidth/2.0f + margin;
    float rightBound = 1.0f - border - carRenderWidth/2.0f - margin;
    if (x < leftBound) x = leftBound;
    if (x > rightBound) x = rightBound;
    
    // Allow car to move vertically within a larger range
    if (y < -0.9f + height/2.0f) y = -0.9f + height/2.0f; // Lower bound
    if (y > 0.0f - height/2.0f) y = 0.0f - height/2.0f;   // Upper bound (keep car in lower half)

    // Update fire cooldown
    if (fireTimer > 0.0f) {
        fireTimer -= deltaTime;
    }

    // Update projectiles - make sure they move up the screen
    for (auto& projectile : projectiles) {
        if (projectile.isActive) {
            // Multiply by deltaTime for frame-rate independence and make sure it moves upward
            projectile.y += PROJECTILE_SPEED * deltaTime;
            
            // Debug projectile position
            if (rand() % 100 == 0) { // Only print occasionally to avoid spamming console
                std::cout << "Projectile at (" << projectile.x << ", " << projectile.y << ")" << std::endl;
            }
            
            // Deactivate if off-screen (top of screen is +1.0)
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
