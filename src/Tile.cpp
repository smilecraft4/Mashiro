#include "Tile.h"
#include "Tile.h"
#include "Tile.h"
#include "Tile.h"
#include "Tile.h"
#include "Canvas.h"

Tile::Tile(const Canvas *canvas, glm::ivec2 position, glm::ivec2 size)
    : _canvas(canvas), _tile_data{{size}, {position}} {

}

Tile::~Tile() {
    glDeleteTextures(1, &_texture_ID);
}

void Tile::Save() {
}

void Tile::Load() {
}

void Tile::Unload() {
}
