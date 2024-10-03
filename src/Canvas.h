#pragma once

#include <vector>
#include <filesystem>

#include "Brush.h"
#include "Tile.h"

class Canvas {
  public:
    Canvas();

    void PaintStroke(const Brush &brush);
    void Render();


    void Save();
    void Open();

  private:
    std::vector<Tile> _tiles;
    std::vector<glm::ivec2> _tiles_pos;
};