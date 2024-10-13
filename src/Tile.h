#pragma once

#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <vector>

class Canvas;

class Tile {
  public:
    Tile(const Tile &) = delete;
    Tile(Tile &&) = delete;
    Tile &operator=(const Tile &) = delete;
    Tile &operator=(Tile &&) = delete;

    Tile(const Canvas *canvas, glm::ivec2 position, glm::ivec2 size);
    ~Tile();

    void Save();
    void Load();
    void Unload();

  private:
    const Canvas *_canvas;
    struct TileData {
        glm::ivec2 _size;
        glm::ivec2 _position;
    } _tile_data;

    std::vector<uint32_t> _pixels;

    GLuint _texture_ID;
    bool _loaded;
};
