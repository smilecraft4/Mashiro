#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glad/glad.h>

class App;

class Viewport {
  public:
    friend class App;

    Viewport(const Viewport &) = delete;
    Viewport(Viewport &&) = delete;
    Viewport &operator=(const Viewport &) = delete;
    Viewport &operator=(Viewport &&) = delete;

    Viewport(const App *app, int width, int height);
    ~Viewport();

    void UpdateView();
    void UpdateProj();
    void CalculateAABB();

    void SetPivot(glm::vec2 pivot);
    void SetSize(glm::ivec2 size, bool update = true);
    void SetPosition(glm::vec2 position, bool update = true);
    void SetZoom(float zoom, bool update = true);
    void SetRotation(float rotation, bool update = true);
    
    float GetZoom() const;
    float GetRotation() const;
    glm::vec2 GetPosition() const;
    glm::ivec2 GetSize() const;
    // glm::ivec4 GetAABB() const;

    glm::mat4 _brush;

  private:
    struct Matrices {
        glm::mat4 _view;
        glm::mat4 _proj;
    } _matrices;


    const App *_app;

    float _zoom;
    float _rotation;
    glm::vec2 _position;
    glm::ivec2 _size;
    glm::vec2 _pivot;
    // TODO: glm::ivec4 _AABB;

    GLuint _matrices_ubo;

    // bool _dirty;
};