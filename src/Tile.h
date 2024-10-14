#pragma once

#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <vector>

class App;
class Canvas;

class Tile {
  public:
    friend class Canvas;

    struct TileData {
        glm::ivec2 _size;
        glm::ivec2 _position;
    };

    
    Tile(const Tile &) = delete;
    Tile &operator=(const Tile &) = delete;
    Tile(Tile &&other) noexcept;
    Tile &operator=(Tile &&other) noexcept;

    /**
     * @brief Unit of the canvas that allow chunk like behaviour (lazy-loading, culling, multithreading, etc...) for near-infinite canvas
     * @param canvas: Parent
     * @param position: Grid position 
     * @param size: Texture dimensions
     */
    Tile(const Canvas *canvas, glm::ivec2 position, glm::ivec2 size);
    ~Tile();
    
    /**
     * @brief Wheter the tile is going to be touched by the brush and or processed in other way
     */
    void SetActive(bool active);

    /**
     * @brief Load the tile from the current file or else create it in the file and load blank data
     */
    void Load();

    /**
     * @brief Unload the texture from opengl, use if the tile as not been seen for a long time and memory is needed elsewhere
     */
    void Unload();

    /**
     * @brief Save the current opengl texture to the current file (use with multithreading once in a while on tiles where progress has been made)
     */
    void Save();

    void BindUniform() const;

    TileData Data() const;
    GLuint TextureID() const;

    void Release();

  private:
    const Canvas *_canvas;

    bool _active;
    bool _loaded;
    bool _culled;
    bool _saved;
    GLuint _texture_ID;
    TileData _tile_data;

    std::vector<std::uint8_t> _pixels;
};
