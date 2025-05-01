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
}; 