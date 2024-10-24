#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>

class Canvas;
class App;
class Tile;

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
    void SetRadiusFactor(float factor);

    float GetRadius() const;
    float GetHardness() const;

    void Use(const Tile *tile) const;

    // void Undo();
    // void Redo();

    struct BrushParameters {
        glm::vec2 _position;
        glm::vec2 _tilt;
        glm::vec4 _color;
        float _pressure;
        float _radius;
        float _hardness;
        float _padding[1];
    } _brush_parameters;

  private:
    const App *_app;

    float _radius = 5.0f;
    float _radius_factor = 1.0f;

    glm::ivec3 _program_work_group_size;
    GLuint _program;
    GLuint _ubo_brush_parameters;
};