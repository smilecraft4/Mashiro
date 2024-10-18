#include "Tile.h"
#include "App.h"
#include "Canvas.h"
#include "File.h"
#include <format>
#include <spdlog/spdlog.h>
#include <string>

// Tile state

Tile::Tile(const Canvas *canvas, File *file, glm::ivec2 position, glm::ivec2 size) : _canvas(canvas), _tile_data() {
    _tile_data._size = size;
    _tile_data._position = position;
    _saved = false;

    Load(file);
}

Tile::~Tile() {
    Release();
}

Tile::Tile(Tile &&other) noexcept {
    _canvas = other._canvas;
    _active = other._active;
    _loaded = other._loaded;
    _culled = other._culled;
    _saved = other._saved;
    _texture_ID = other._texture_ID;
    _tile_data = other._tile_data;

    other._canvas = nullptr;
    other._active = false;
    other._loaded = false;
    other._culled = false;
    other._saved = false;
    other._texture_ID = 0;
    other._tile_data = TileData();
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
    if (active) {
        _saved = false;
    }
}

void Tile::Save(File *file) {
    if (_saved) {
        spdlog::warn("Tile_{}_{} is already saved", _tile_data._position.x, _tile_data._position.y);
        return;
    }
    // TODO add multithreading
    spdlog::warn("Saving Tile_{}_{}", _tile_data._position.x, _tile_data._position.y);

    auto pixels = std::vector<std::uint32_t>(_tile_data._size.x * _tile_data._size.y);
    glBindTexture(GL_TEXTURE_2D, _texture_ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Update texture in the file

    // Auto-save file using another thread
    file->SaveTexture(_tile_data._position.x, _tile_data._position.y, pixels, _canvas->_app->_settings->_file_compression);
    _saved = true;
}

void Tile::Load(File *file) {
    spdlog::warn("Loading Tile_{}_{}", _tile_data._position.x, _tile_data._position.y);

    const auto raw = file->GetTexture(_tile_data._position.x, _tile_data._position.y);
    std::vector<uint32_t> tile_pixels;
    if (raw.has_value()) {
        tile_pixels = raw.value();
    } else {
        tile_pixels = std::vector<uint32_t>(_tile_data._size.x * _tile_data._size.y, 0xFFFFFFFF);
    }

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

    _saved = true;
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
