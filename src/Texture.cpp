#include "../include/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include <iostream>

Texture::Texture() : textureID(0), width(0), height(0), channels(0) {
}

Texture::~Texture() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
}

bool Texture::loadFromFile(const std::string& filename) {
    stbi_set_flip_vertically_on_load(true); // Ensure images are loaded with OpenGL's expected orientation
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return false;
    }

    std::cout << "Loaded texture " << filename << " with " << channels << " channels" << std::endl;

    // Generate and bind texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data - properly handle various color formats
    GLenum format;
    if (channels == 4) {
        format = GL_RGBA;
        std::cout << "Using RGBA format for " << filename << std::endl;
    } else if (channels == 3) {
        format = GL_RGB;
        std::cout << "Using RGB format for " << filename << std::endl;
    } else if (channels == 1) {
        format = GL_RED;
        std::cout << "Using RED format for " << filename << std::endl;
    } else {
        format = GL_RGB;
        std::cout << "Using default RGB format for " << filename << " (unexpected channel count: " << channels << ")" << std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Free image data
    stbi_image_free(data);

    return true;
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
} 