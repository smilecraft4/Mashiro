#include "Canvas.h"
#include "App.h"
#include "File.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <set>
#include <spdlog/spdlog.h>
#include <string>

void Canvas::LoadFile() {
    // clear everying
    _tiles.clear();
    _tiles_index.clear();

    const auto tile_to_load = _app->_file->GetSavedTileLocation();
    _tiles.reserve(tile_to_load.size());
    for (size_t i = 0; i < tile_to_load.size(); i++) {
        _tiles_index.emplace(tile_to_load[i], _tiles.size());
        _tiles.push_back(Tile(this, _app->_file.get(), {tile_to_load[i].first, tile_to_load[i].second},
                              glm::ivec2(_app->_file->GetTileResolution())));
    }
}

Canvas::Canvas(App *app, glm::ivec2 tiles_size) : _app(app), _tiles_size(tiles_size) {
    // Compile shader
    _tiles_program = glCreateProgram();
    std::string program_name = "Tiles Program";
    glObjectLabel(GL_PROGRAM, _tiles_program, program_name.size(), program_name.c_str());

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const auto canvas_vert_file = _app->_data.open("shaders/canvas.vert");
    std::string_view canvas_vert(canvas_vert_file.begin(), canvas_vert_file.end());
    const char *canvas_vert_source = canvas_vert.data();
    glShaderSource(vertex_shader, 1, &canvas_vert_source, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto canvas_frag_file = _app->_data.open("shaders/canvas.frag");
    std::string_view canvas_frag(canvas_frag_file.begin(), canvas_frag_file.end());
    const char *canvas_frag_source = canvas_frag.data();
    glShaderSource(fragment_shader, 1, &canvas_frag_source, nullptr);
    glCompileShader(fragment_shader);

    glAttachShader(_tiles_program, vertex_shader);
    glAttachShader(_tiles_program, fragment_shader);
    glLinkProgram(_tiles_program);

    GLint isLinked = 0;
    glGetProgramiv(_tiles_program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(_tiles_program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(_tiles_program, maxLength, &maxLength, &infoLog[0]);
        spdlog::critical("{}", infoLog.data());
    }
    assert(isLinked && "Fail to linked program");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Create TileData uniform
    glGenBuffers(1, &_tiles_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _tiles_ubo);
    std::string ubo_matrices_name = "Tiles Uniform Buffer";
    glObjectLabel(GL_BUFFER, _tiles_ubo, ubo_matrices_name.size(), ubo_matrices_name.c_str());
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Tile::TileData), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, _tiles_ubo, 0, sizeof(Tile::TileData));

    // Create vertex array
    glGenVertexArrays(1, &_tiles_mesh); 
    assert(_tiles_mesh && "Failed to create canvas vertex array object");
    glBindVertexArray(_tiles_mesh);
    std::string tiles_mesh_name = "Tiles Mesh";
    glObjectLabel(GL_VERTEX_ARRAY, _tiles_mesh, tiles_mesh_name.size(), tiles_mesh_name.c_str());
}

Canvas::~Canvas() {
    glDeleteBuffers(1, &_tiles_ubo);
    glDeleteVertexArrays(1, &_tiles_mesh);
    glDeleteProgram(_tiles_program);
}

void Canvas::Process(const Brush *brush) {
    for (size_t i = 0; i < _tiles.size(); i++) {
        if (_tiles[i]._active) {
            brush->Use(&_tiles[i]);
        }
    }
}

void Canvas::Render() {
    CullTiles();
    RenderTiles();
}

void Canvas::SaveTiles() {
    for (auto &tile : _tiles) {
        if (!tile._saved) {
            tile.Save(_app->_file.get());        
        }
    }
}

void Canvas::UpdateTilesProcessed(std::vector<glm::vec2> brush_path, float range) {
    // Retrieve where the Tool is going to work
    // const auto tool_work_region = _app->GetToolWorkRegion() // returns the path coordinates where the Tool is going
    // to work;

    // Find the tiles that are going to be processed
    std::set<std::pair<int, int>> processed_id;
    for (size_t i = 0; i < brush_path.size(); i++) {
        // extends this path to include all tiles that are at a distance
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                const glm::ivec2 coord = glm::floor(brush_path[i] / glm::vec2(_tiles_size));
                const std::pair<int, int> id = {coord.x + x, coord.y + y};
                processed_id.insert(id);
            }
        }
    }

    // Load the necessary tiles if they do not exists
    for (auto const &id : processed_id) {
        if (!_tiles_index.contains(id)) {
            size_t index = _tiles.size();
            _tiles_index.emplace(id, index);

            _tiles.emplace_back(Tile(this, _app->_file.get(), {id.first, id.second}, _tiles_size));
        }
    }

    // Update tiles processing state
    for (auto const &[id, index] : _tiles_index) {
        if (processed_id.contains(id)) {
            _tiles[index].SetActive(true);
        } else {
            _tiles[index].SetActive(false);
        }
    }
}

bool Canvas::GetTileIndex(glm::ivec2 position, size_t &index) {
    const std::pair<int, int> key = {position.x, position.y};

    if (_tiles_index.contains(key)) {
        index = _tiles_index[key];
        return true;
    } else {
        index = SIZE_MAX;
        return false;
    }
}

void Canvas::RenderTiles() {
    glUseProgram(_tiles_program);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_tiles_mesh);

    for (auto const &tile : _tiles) {
        tile.BindUniform();

        if (tile._active) {
            glUniform3f(glGetUniformLocation(_tiles_program, "tint"), 1.0f, 0.0f, 0.0f);
        } else if (tile._loaded) {
            glUniform3f(glGetUniformLocation(_tiles_program, "tint"), 0.0f, 1.0f, 0.0f);
        } else {
            glUniform3f(glGetUniformLocation(_tiles_program, "tint"), 0.0f, 0.0f, 1.0f);
        }

        glBindTexture(GL_TEXTURE_2D, tile._texture_ID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
void Canvas::CullTiles() {
    for (auto const &tile : _tiles) {
        // If the viewport AABB does the AABB of this tile
        // set this tile has not culled
        // otherwise set it as culled

        // unload the tile tile.Unload();
    }
}
