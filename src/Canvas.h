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

    glm::ivec2 GetTileUnderCursor(glm::vec2 cursor_pos);

    void UpdateCursorPos(glm::vec2 cursor_pos);

    size_t CreateTile(glm::ivec2 pos, glm::ivec2 size);

    void LoadTile(size_t index);
    void UnloadTile(size_t index);
    void SaveTile(size_t index);
    void ClearTile(size_t index);

    // void Load(std::filesystem::path filename);
    // void Save(std::filesystem::path filename);
    struct TileData {
        glm::ivec2 _size;
        glm::ivec2 _position;
        glm::mat4 _model;
    } _tile_data;
    GLuint _ubo_tile_data;

    std::map<int, size_t> _tiles_elements;
    std::vector<bool> _tiles_loaded;
    std::vector<TileData> _tiles_datas;
    std::vector<GLuint> _tiles_textures;

  private:


    GLuint _mesh;
    GLuint _program;
    GLuint _texture;

    std::vector<std::uint32_t> _pixels;

    const App *_app;
};