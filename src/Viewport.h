#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class Viewport {
  public:
    Viewport(){};

    void SetPosition(glm::vec2 position = glm::vec2(0.0f, 0.0f)) noexcept {};
    void SetRotation(float rotation = 0.0f) noexcept {};
    void SetZoom(float zoom = 1.0f) noexcept {};

    void AddPosition(glm::vec2 position = glm::vec2(0.0f, 0.0f)) noexcept {};
    void AddRotation(float rotation = 0.0f) noexcept {};
    void AddZoom(float zoom = 1.0f) noexcept {};

  private:
    void UpdateViewMat() noexcept {};

  private:
    glm::vec2 _pos;
    float _zoom; // optional
    float _rotation;

    glm::mat4 _view_mat;
};