#pragma once

#include <glm/vec2.hpp>

// TODO: Implement RealTimeStylus.h

class Stylus {
  public:
    Stylus(const Stylus &) = delete;
    Stylus(Stylus &&) = delete;
    Stylus &operator=(const Stylus &) = delete;
    Stylus &operator=(Stylus &&) = delete;

    Stylus();
    ~Stylus();

  protected:

  private:
    glm::vec2 _position;
    glm::vec2 _velocity;
    float _pressure;
    float _tilt;
};