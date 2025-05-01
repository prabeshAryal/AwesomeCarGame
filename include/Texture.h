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