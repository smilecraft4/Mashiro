#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>

class Canvas;
class App;

class Brush {
  public:
    Brush(const Brush &) = delete;
    Brush(Brush &&) = delete;
    Brush &operator=(const Brush &) = delete;
    Brush &operator=(Brush &&) = delete;

    Brush(const App* app);
    ~Brush();

    void SetPosition(glm::vec2 position, bool update = true);
    void SetColor(glm::vec4 color, bool update = true);
    void SetHardness(float hardness, bool update = true);
    void SetRadius(float radius, bool update = true);

    float GetRadius() const;
    float GetHardness() const;

    void Use(const Canvas *canvas);

    // void Undo();
    // void Redo();

  private:
    const App *_app;

    struct BrushParameters {
        glm::vec2 _position;
        glm::vec2 _tilt;
        glm::vec4 _color;
        float _pressure;
        float _radius;
        float _hardness;
        float _padding[1];
    } _brush_parameters;

    glm::ivec3 _program_work_group_size;
    GLuint _program;
    GLuint _ubo_brush_parameters;
};