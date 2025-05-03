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
#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include "Car.h"
#include "Texture.h"

class Level {
public:
    enum class ObstacleType {
        BARRIER,
        CONE,
        ROCK,
        TREE,
        ENEMY_CAR
    };

    struct Obstacle {
        float x, y;
        float width;  // Will be set in spawnObstacle
        float height; // Will be set in spawnObstacle
        ObstacleType type;
        bool isActive;
    };

    Level();
    ~Level();

    void init();
    void update(float deltaTime, Car& playerCar);
    void render();
    bool isComplete() const;
    int getScore() const;
    int getLevel() const;
    int getHighScore() const;
    void reset();
    void checkCollisions(Car& playerCar);

private:
    void createRoadTexture();
    void spawnObstacle();
    void spawnEnemyCar();
    void drawRoad();
    void drawLaneLines();
    void drawObstacles();
    void updateLevel();
    void updateTimerScore(float deltaTime);
    void loadHighScore();
    void saveHighScore();

    GLuint roadTextureId;
    std::vector<Obstacle> obstacles;
    std::vector<Car> enemyCars;
    float roadWidth;
    float roadLength;
    float scrollSpeed;
    int score;
    int level;
    float levelTimer;
    float obstacleSpawnTimer;
    float enemySpawnTimer;
    float roadOffset;  // For scrolling effect
    float scoreTimer;  // Timer for score accumulation
    Texture barrierTexture;
    Texture coneTexture;
    Texture rockTexture;
    Texture treeTexture;
    Texture enemyCarTexture;
    static constexpr float LEVEL_1_SCORE = 0.0f;
    static constexpr float LEVEL_2_SCORE = 100.0f;
    static constexpr float LEVEL_3_SCORE = 300.0f;
    static constexpr int OBSTACLE_POINTS = 50;
    static constexpr int ENEMY_CAR_POINTS = 200;
    static constexpr int TIME_SCORE_POINTS = 1;  // Points per second (changed from 100 to 1)
    static constexpr float TIME_SCORE_INTERVAL = 1.0f;  // Score every second
    static constexpr float OBSTACLE_SPAWN_INTERVAL = 1.0f;
    static constexpr float ENEMY_SPAWN_INTERVAL = 5.0f;
    static constexpr int ROAD_TEXTURE_SIZE = 256;  // Size of the procedural texture
    int highScore;
}; 
//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by NeedForSpeedInfinity.rc

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        101
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

class Texture {
private:
    GLuint textureID;
    int width;
    int height;
    int channels;

public:
    Texture();
    ~Texture();

    bool loadFromFile(const std::string& filename);
    void bind() const;
    void unbind() const;
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
}; 
