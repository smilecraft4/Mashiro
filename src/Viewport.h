#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class App;

class Viewport {
  public:
    Viewport(const Viewport &) = delete;
    Viewport(Viewport &&) = delete;
    Viewport &operator=(const Viewport &) = delete;
    Viewport &operator=(Viewport &&) = delete;

    Viewport(const App *app);
    ~Viewport();

    void UpdateView();
    void CalculateAABB();

    void SetPosition(glm::vec2 position, bool update = true);
    void SetZoom(float zoom, bool update = true);
    void SetRotation(float rotation, bool update = true);
    
    glm::vec2 GetPosition() const;
    float GetZoom() const;
    float GetRotation() const;
    glm::ivec4 GetAABB() const;

    glm::mat4 _viewport;

  private:
    float _zoom;
    float _rotation;
    glm::vec2 _position;
    glm::ivec4 _AABB;
    const App *_app;
    bool _dirty;
};