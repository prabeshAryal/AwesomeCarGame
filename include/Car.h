#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include "Texture.h"

class Car {
public:
    struct Projectile {
        float x, y;
        float speed;
        bool isActive;
    };

    Car();
    ~Car();

    void init();
    void initAsEnemy();
    void update(float deltaTime);
    void render() const;
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void stopHorizontal();
    void stopVertical();
    void fire();
    bool checkCollision(float x, float y, float width, float height) const;
    bool checkProjectileCollision(float x, float y, float width, float height) const;
    void takeDamage(float amount);
    void addScore(int points);
    float getHealth() const;
    int getScore() const;
    float getX() const;
    float getY() const;
    float getWidth() const;
    float getHeight() const;
    float getSpeed() const;
    bool isActive() const;
    void setX(float x);
    void setY(float y);
    void setSpeed(float speed);
    void setActive(bool active);
    const std::vector<Projectile>& getProjectiles() const;
    static void ensureEnemyCarTextureLoaded();

private:
    float x, y;
    const float width = 0.1f;
    const float height = 0.1f;
    float speed;
    float health;
    int score;
    bool active;
    bool isEnemy = false;
    const float fireCooldown;
    float fireTimer;
    Texture carTexture;
    std::vector<Projectile> projectiles;
    static constexpr float FIRE_COOLDOWN = 0.5f;
    static constexpr float PROJECTILE_SPEED = 10.0f;
    float horizontalSpeed;
    float verticalSpeed;
    static Texture enemyCarTexture;
    static bool enemyCarTextureLoaded;
}; 