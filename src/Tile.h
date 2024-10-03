#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>

#include "Brush.h"
#include "Texture.h"

class Tile {
  public:
    Tile(){};

    void PaintBrush(const Brush *brush){};

  private:
    glm::ivec2 _position;
    Texture _texture;

    bool _loaded;
};