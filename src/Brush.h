#pragma once

#include <functional>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "Texture.h"

class Brush {
  public:
    Brush();

  private:
    glm::vec2 _position;
    glm::vec4 _color;
    float _size;
    float _pressure;
    float _tilt;
    float _step;
    Texture _alpha;
};