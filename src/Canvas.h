#pragma once

#include <filesystem>
#include <map>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class App;

class Canvas {
  public:
    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&) = delete;
    Canvas &operator=(const Canvas &) = delete;
    Canvas &operator=(Canvas &&) = delete;

    Canvas(const App *app);
    ~Canvas();

    void Render() const;
    void SetPosition(glm::vec2 position, bool update = true);
    void UpdateModel();

    // void Load(std::filesystem::path filename);
    // void Save(std::filesystem::path filename);

  private:
    glm::vec2 _position;
    glm::mat4 _model;
    glm::ivec2 _size;

    GLuint _mesh;
    GLuint _program;
    GLuint _texture;

    const App *_app;
};