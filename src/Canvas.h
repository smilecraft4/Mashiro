#pragma once

#include <filesystem>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <map>

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

    glm::ivec2 Size() const;
    GLuint TextureID() const;

    // void LoadTile(glm::ivec2 position);
    // void UnloadTile(glm::ivec2 position);
    // void SaveTile(glm::ivec2 position);
    // void ClearTile(glm::ivec2 position);

    // void Load(std::filesystem::path filename);
    // void Save(std::filesystem::path filename);

  private:
    struct TileData {
        glm::ivec2 _size;
        glm::ivec2 _position;
        glm::mat4 _model;
    } _tile_data;

    std::map<glm::ivec2, size_t> _tiles_elements;
    std::vector<bool> _tiles_loaded;
    std::vector<glm::ivec4> _tiles_AABB;
    std::vector<TileData> _tiles_datas;
    std::vector<GLuint> _tiles_textures;

    GLuint _mesh;
    GLuint _program;
    GLuint _texture;

    GLuint _ubo_tile_data;

    const App *_app;
};