#include "Tile.h"
#include "App.h"
#include "Canvas.h"
#include "Tile.h"
#include <format>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>

// Tile state

Tile::Tile(const Canvas *canvas, glm::ivec2 position, glm::ivec2 size) : _canvas(canvas), _tile_data() {
    _tile_data._size = size;
    _tile_data._position = position;

    Load();
}

Tile::~Tile() {
    Release();
}

Tile::Tile(Tile &&other) noexcept{
    _canvas = other._canvas;
    _active = other._active;
    _loaded = other._loaded;
    _culled = other._culled;
    _saved = other._saved;
    _texture_ID = other._texture_ID;
    _tile_data = other._tile_data;
    _pixels = other._pixels;

    other._canvas = nullptr;
    other._active = false;
    other._loaded = false;
    other._culled = false;
    other._saved = false;
    other._texture_ID = 0;
    other._tile_data = TileData();
    other._pixels.clear();
}

Tile &Tile::operator=(Tile &&other) noexcept {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

void Tile::BindUniform() const {
    glBindBuffer(GL_UNIFORM_BUFFER, _canvas->_tiles_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TileData), &_tile_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Tile::SetActive(bool active) {
    _active = active;
}

void Tile::Save() {
    if (_saved) {
        spdlog::trace("Tile_{}_{} is already saved", _tile_data._position.x, _tile_data._position.x);
        return;
    }
    // TODO add multithreading
    spdlog::trace("Saving Tile_{}_{}", _tile_data._position.x, _tile_data._position.x);
    _pixels = std::vector<std::uint8_t>(_tile_data._size.x * _tile_data._size.y * 4);
    glGetTexImage(_texture_ID, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels.data());
    // Update texture in the file
    // Auto-save file using another thread
}

void Tile::Load() {
    spdlog::trace("Loading Tile_{}_{}", _tile_data._position.x, _tile_data._position.x);
    // TODO add multithreading
    // find grid in the file
    // if it exist load the pixel
    // const std::vector<uint32_t> tile_pixels = _app->_file->GetTilePixels(_tile_data._position)
    // else load a blank pixel
    const std::vector<uint32_t> tile_pixels(_tile_data._size.x * _tile_data._size.y, 0x00000000);
    // Create the texture
    glGenTextures(1, &_texture_ID);
    glBindTexture(GL_TEXTURE_2D, _texture_ID);
    std::string text = std::format("Canvas Texture ({},{})", _tile_data._position.x, _tile_data._position.y).c_str();
    glObjectLabel(GL_TEXTURE, _texture_ID, text.size(), text.c_str());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _tile_data._size.x, _tile_data._size.y);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _tile_data._size.x, _tile_data._size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                    tile_pixels.data());

    // Is this necesseary
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Tile::Unload() {
    spdlog::trace("Unloading Tile_{}_{}", _tile_data._position.x, _tile_data._position.x);
}

Tile::TileData Tile::Data() const {
    return _tile_data;
}

GLuint Tile::TextureID() const {
    return _texture_ID;
}

void Tile::Release() {
    glDeleteTextures(1, &_texture_ID);
    _texture_ID = 0;
}
