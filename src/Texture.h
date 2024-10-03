#pragma once
#include <filesystem>

#include <glad/glad.h>

class Texture {
  public:
    Texture(){};
    Texture(int width, int height, GLenum format){};
    ~Texture(){};

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    Texture(Texture &&other) noexcept {};
    Texture &operator=(Texture &&other) noexcept {};

    void Release() noexcept {};

    GLuint _ID = 0;
};