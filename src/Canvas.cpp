#include "Canvas.h"
#include "App.h"
#include <format>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <string>

Canvas::Canvas(const App *app) : _app(app), _tile_data() {
    _tile_data._size = {256, 256};
    _tile_data._position = {0, 0};
    UpdateModel();

    // Compile shader
    _program = glCreateProgram();
    const GLchar program_name[] = "Canvas Program";
    glObjectLabel(GL_PROGRAM, _program, sizeof(program_name), program_name);

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

    glAttachShader(_program, vertex_shader);
    glAttachShader(_program, fragment_shader);
    glLinkProgram(_program);

    GLint isLinked = 0;
    glGetProgramiv(_program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(_program, maxLength, &maxLength, &infoLog[0]);
        spdlog::critical("{}", infoLog.data());
    }
    assert(isLinked && "Fail to linked program");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Create TileData uniform
    glGenBuffers(1, &_ubo_tile_data);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_tile_data);
    const GLchar ubo_matrices_name[] = "Canvas TileData Uniform Buffer";
    glObjectLabel(GL_BUFFER, _ubo_tile_data, sizeof(ubo_matrices_name), ubo_matrices_name);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(TileData), &_tile_data, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 2, _ubo_tile_data, 0, sizeof(TileData));

    _pixels = std::vector<std::uint32_t>(_tile_data._size.x * _tile_data._size.y, 0xFF0000FF);

    // Create texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    const GLchar texture_name[] = "Canvas Texture";
    glObjectLabel(GL_TEXTURE, _texture, sizeof(texture_name), texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const std::vector<std::uint32_t> pixels(_tile_data._size.x * _tile_data._size.y, 0xFF0000FF);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _tile_data._size.x, _tile_data._size.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _tile_data._size.x, _tile_data._size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Create vertex array
    glGenVertexArrays(1, &_mesh);
    assert(_mesh && "Failed to create canvas vertex array object");
    glBindVertexArray(_mesh);
    const GLchar mesh_name[] = "Canvas VertexArray";
    glObjectLabel(GL_VERTEX_ARRAY, _mesh, sizeof(mesh_name), mesh_name);
}

Canvas::~Canvas() {
    for (size_t i = 0; i < _tiles_textures.size(); i++) {
        glDeleteTextures(1, &_tiles_textures[i]);
    }

    glDeleteVertexArrays(1, &_mesh);
    glDeleteTextures(1, &_texture);
    glDeleteProgram(_program);
}

void Canvas::Render() const {
    glUseProgram(_program);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_mesh);

    for (size_t i = 0; i < _tiles_textures.size(); i++) {
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo_tile_data);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TileData), &_tiles_datas[i]);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindTexture(GL_TEXTURE_2D, _tiles_textures[i]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void Canvas::UpdateModel() {
    _tile_data._model = glm::mat4(1.0f);
    _tile_data._model = glm::scale(_tile_data._model, glm::vec3(_tile_data._size, 1.0f));
    _tile_data._model = glm::translate(_tile_data._model, glm::vec3(_tile_data._position, 0.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_tile_data);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TileData), &_tile_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::ivec2 Canvas::Size() const {
    return _tile_data._size;
}

GLuint Canvas::TextureID() const {
    return _texture;
}

glm::ivec2 Canvas::GetTileUnderCursor(glm::vec2 cursor_pos) {
    glm::ivec2 t = glm::floor((glm::vec2(cursor_pos) + glm::vec2(_tile_data._size / 2)) / glm::vec2(_tile_data._size));
    return t;
}

void Canvas::UpdateCursorPos(glm::vec2 cursor_pos) {

    auto tile_pos = GetTileUnderCursor(cursor_pos);
    // spdlog::info("!!!!\tCursor is at tile at ({},{})", tile_pos.x, tile_pos.y);

    for (size_t i = 0; i < _tiles_loaded.size(); i++) {
        if (_tiles_loaded[i]) {
            const auto &data = _tiles_datas[i];
            if ((tile_pos.x - 1) > data._position.x || (tile_pos.x + 1) < data._position.x ||
                (tile_pos.y - 1) > data._position.y || (tile_pos.y + 1) < data._position.y) {
                // spdlog::info("\t-\tUnloaded tile at ({},{})", data._position.x, data._position.y);
                UnloadTile(i);
            }
        }
    }

    for (int x = tile_pos.x - 1; x < tile_pos.x + 2; x++) {
        for (int y = tile_pos.y - 1; y < tile_pos.y + 2; y++) {
            glm::ivec2 tile(x, y);
            if (!_tiles_elements.contains(x + y * _tile_data._size.x)) {
                _tiles_elements.emplace(x + y * _tile_data._size.x, CreateTile({x, y}, _tile_data._size));
                // spdlog::info("NEW: Created new tile at ({},{})", x, y);
            }
            // spdlog::info("\t+\tLoaded tile tile at ({},{})", x, y);
            LoadTile(_tiles_elements[x + y * _tile_data._size.x]);
        }
    }
}

size_t Canvas::CreateTile(glm::ivec2 pos, glm::ivec2 size) {
    size_t index = _tiles_datas.size();

    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(size, 1.0f));
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    _tiles_datas.push_back({size, pos, model});
    _tiles_textures.push_back(0);
    _tiles_loaded.push_back(false);

    glGenTextures(1, &_tiles_textures[index]);
    glBindTexture(GL_TEXTURE_2D, _tiles_textures[index]);
    std::string text = std::format("Canvas Texture ({},{})", pos.x, pos.y).c_str();
    glObjectLabel(GL_TEXTURE, _tiles_textures[index], text.size(), text.c_str());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _tile_data._size.x, _tile_data._size.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _tile_data._size.x, _tile_data._size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                    _pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    return index;
}

void Canvas::LoadTile(size_t index) {

    _tiles_loaded[index] = true;
}

void Canvas::UnloadTile(size_t index) {
    _tiles_loaded[index] = false;
}

void Canvas::SaveTile(size_t index) {
}

void Canvas::ClearTile(size_t index) {
}

void Canvas::SetPosition(glm::vec2 position, bool update) {
    _tile_data._position = position;
    if (update) {
        UpdateModel();
    }
}
