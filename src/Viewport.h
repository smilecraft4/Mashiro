#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class App;

class Viewport {
  public:
    Viewport(const Viewport &) = delete;
    Viewport(Viewport &&) = delete;
    Viewport &operator=(const Viewport &) = delete;
    Viewport &operator=(Viewport &&) = delete;

    Viewport(const App* app);
    ~Viewport();

    void SetPosition(glm::vec2 position, bool update = true);
    void SetZoom(float zoom, bool update = true);
    void SetRotation(float rotation, bool update = true);

    void UpdateView();

  private:
    float _zoom;
    float _rotation;
    glm::vec2 _position;
    glm::mat4 _viewport;
    const App *_app;
    bool _dirty;
};