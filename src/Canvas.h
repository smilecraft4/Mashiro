#pragma once

#include <filesystem>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <map>

#include "Tile.h"
#include "Brush.h"

class App;
class Tool;
class File;

class Canvas {
  public:
    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&) = delete;
    Canvas &operator=(const Canvas &) = delete;
    Canvas &operator=(Canvas &&) = delete;

    void LoadFile();
    Canvas(App *app, glm::ivec2 tiles_size = {512, 512});
    ~Canvas();

    /**
     * @brief Manage Loading/Unloading/Saving of the canvas tiles
     */
    void UpdateTilesProcessed(std::vector<glm::vec2> brush_path, float range);

    /**
     * @brief Process tiles (paint, fill, select, erase, etc...) the canvas using a tool
     * @param Tool (BrushTool, FillTool, Eraser, ColorPicker, etc..)
     */
    void Process(const Brush *brush);

    /**
     * @brief Render canvas
     */
    void Render();

    /**
     * @brief Save all the tiles that are unsaved
     */
    void SaveTiles();

    GLuint _tiles_ubo;
    glm::ivec2 _tiles_size;
    App *_app;
  protected:
    /**
     * @brief Disable rendering of tiles that are not visible by the viewport, that is, the user
     */
    void CullTiles();

    /**
     * @brief Render the tiles to the user
     */
    void RenderTiles();

    /**
     * @param: index index of the tile
     * @return if the tile exists or not
     */
    bool GetTileIndex(glm::ivec2 position, size_t &index);

  private:

    GLuint _tiles_mesh;
    GLuint _tiles_program;

    std::map<std::pair<int, int>, size_t> _tiles_index;
    std::vector<Tile> _tiles;
};