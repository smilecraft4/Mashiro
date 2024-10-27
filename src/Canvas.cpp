#include "Canvas.h"
#include "App.h"
#include "Log.h"
#include "Preferences.h"
#include "Viewport.h"

std::vector<uint32_t> Canvas::_pixels;
std::unique_ptr<Uniformbuffer> Canvas::_tile_ubo;
std::unique_ptr<Program> Canvas::_program;
std::unique_ptr<Mesh> Canvas::_mesh;

void Canvas::Init() {
    _tile_ubo = Uniformbuffer::Create(TEXT("Tile Uniformbuffer"), 1, sizeof(Tile), nullptr);

    _program = Program::Create(TEXT("Tile Uniformbuffer"));
    _program->AddShader("data/tile.vert", GL_VERTEX_SHADER);
    _program->AddShader("data/tile.frag", GL_FRAGMENT_SHADER);
    _program->Compile();

    const auto tile_resolution = Preferences::Get()->_tile_resolution;
    const auto tile_color = Preferences::Get()->_tile_default_color;

    _pixels = std::vector<uint32_t>(tile_resolution * tile_resolution, tile_color);

    _mesh = Mesh::Create(TEXT("Tile Uniformbuffer"));
}

Canvas::Canvas() {
}

Canvas::~Canvas() {
}

std::unique_ptr<Canvas> Canvas::New() {
    auto canvas = std::make_unique<Canvas>();
    canvas->_saved = true;
    return canvas;
}

void Canvas::Load(glm::ivec2 coord, File *file) {
    if (_coord_tile.contains({coord.x, coord.y})) {
        return;
    }

    CreateTile(coord);
    if (file) {
        auto pixels = file->ReadTileTexture(coord.x, coord.y);
        _tiles_textures[_coord_tile[{coord.x, coord.y}]].SetPixels(pixels);
    } else {
        _tiles_textures[_coord_tile[{coord.x, coord.y}]].SetPixels(_pixels);
    }
}

std::unique_ptr<Canvas> Canvas::Open(File *file) {
    auto canvas = std::make_unique<Canvas>();

    for (const auto &[x, y] : file->GetSavedTileLocation()) {
        canvas->Load({x, y}, file);
    }

    canvas->_saved = true;

    return canvas;
}

bool Canvas::IsSaved() const {
    return _saved;
}

void Canvas::Save(File *file) {
    for (size_t i = 0; i < _tiles_saved.size(); i++) {
        if (!_tiles_saved[i]) {
            SaveTile(i, file);
        }
    }

    _saved = true;
}

void Canvas::LazyLoad(glm::vec2 cursor, File *file) {
    const auto tile_resolution = Preferences::Get()->_tile_resolution;

    // get the current cursor tile coord
    const glm::ivec2 coord = glm::floor(cursor / glm::vec2(tile_resolution));
    Load(coord, file);

    // See if the tile at tile coord + radius (AABB test ??) is loaded around the cursor
    //	if the tile is not create see if it exist on the file
    //	if it doesn't exist on disk create it
}

void Canvas::LazySave(glm::vec2 cursor, File *file) {
    // if the cursor is not visited from a long time
    const auto max_save = Preferences::Get()->_lazy_save_count;

    int save_counter = 0;
    for (size_t i = 0; i < _tiles_saved.size(); i++) {
        if (!_tiles_saved[i] && !_tiles_processing[i] && !_tiles_visibility[i]) {
            // SaveTile function
            SaveTile(i, file);
            save_counter++;
            if (save_counter >= max_save) {
                break;
            }

            // When all the tiles have been saved mark the file as saved
            if (i == _tiles_saved.size() - 1) {
                _saved = true;
            }
        }
    }
}

void Canvas::SaveTile(size_t i, File *file) {
    const auto x = _tiles_data[i].coord.x;
    const auto y = _tiles_data[i].coord.y;
    auto pixels = _tiles_textures[i].ReadPixels();

    file->WriteTileTexture(x, y, pixels);

    _tiles_saved[i] = true;
}

void Canvas::Refresh() {
    _program->Compile();
}

void Canvas::Paint(Brush *brush) {
    const auto tile_resolution = Preferences::Get()->_tile_resolution;
    const glm::ivec2 coord = glm::floor(brush->GetPosition() / glm::vec2(tile_resolution));

    for (int y = -1; y < 2; y++) {
        for (int x = -1; x < 2; x++) {
            Load({coord.x + x, coord.y + y}, nullptr);
            const auto index = _coord_tile[{coord.x + x, coord.y + y}];
            _tiles_processing[index] = true;
            _tile_ubo->SetData(0, sizeof(Tile), &_tiles_data[index]);
            brush->Paint(&_tiles_textures[index]);
            _tiles_processing[index] = false;
            _tiles_saved[index] = false;
        }
    }

    // Consider optimizing this to avoid needless save when no data was written
    _saved = false;
    SetWindowText(App::Get()->_window->Hwnd(), App::Get()->_file->GetDisplayName().c_str());
}

void Canvas::Render(Viewport *viewport) {
    CullTiles(viewport);
    RenderTiles();
}

void Canvas::CreateTile(glm::ivec2 coord) {
    if (_coord_tile.contains({coord.x, coord.y})) {
        Log::Trace(std::format(TEXT("Tile ({},{}) is already created"), coord.x, coord.y));
        return;
    }

    const auto resolution = Preferences::Get()->_tile_resolution;

    size_t index = _tiles_data.size();

    _coord_tile.emplace(std::pair<int, int>(coord.x, coord.y), index);
    _tiles_data.push_back(Tile(coord, 0, resolution));
    _tiles_aabb.push_back(AABB({0.0f, 0.0f}, {1.0f, 1.0f}));
    _tiles_visibility.push_back(false);
    _tiles_saved.push_back(false);
    _tiles_processing.push_back(false);
    _tiles_textures.push_back(
        Texture(std::format(TEXT("Tile ({},{}) Texture"), coord.x, coord.y), resolution, resolution));

    Log::Trace(std::format(TEXT("Created Tile ({},{})"), coord.x, coord.y));

    _saved = false;
}

void Canvas::DeleteTile(glm::ivec2 coord) {
    if (!_coord_tile.contains({coord.x, coord.y})) {
        Log::Trace(std::format(TEXT("Tile ({},{}) does not exist and thus cannot be deleted"), coord.x, coord.y));
        return;
    }

    size_t index = _coord_tile[{coord.x, coord.y}];

    _tiles_data.erase(_tiles_data.begin() + index);
    _tiles_aabb.erase(_tiles_aabb.begin() + index);
    _tiles_visibility.erase(_tiles_visibility.begin() + index);
    _tiles_saved.erase(_tiles_saved.begin() + index);
    _tiles_textures.erase(_tiles_textures.begin() + index);
    _tiles_processing.erase(_tiles_processing.begin() + index);
    _coord_tile.erase({coord.x, coord.y});

    Log::Trace(std::format(TEXT("Deleted Tile ({},{})"), coord.x, coord.y));

    _saved = false;
}

void Canvas::ReloadTile(glm::ivec2 coord) {
    // ?? what do i do here ???
}

void Canvas::RenderTiles() {
    _program->Bind();
    for (size_t i = 0; i < _tiles_data.size(); i++) {
        if (_tiles_visibility[i]) {
            _tile_ubo->SetData(0, sizeof(Tile), &_tiles_data[i]);
            _tiles_textures[i].Bind(0);
            _mesh->Render(GL_TRIANGLES, 6);
        }
    }
}

void Canvas::CullTiles(Viewport *viewport) {
    for (size_t i = 0; i < _tiles_aabb.size(); i++) {
        _tiles_visibility[i] = viewport->IsVisible(_tiles_aabb[i]);
    }
}
