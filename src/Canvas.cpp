#include "Canvas.h"
#include "App.h"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

Canvas::Canvas(const App *app) : _app(app), _size(512, 512), _position(0.0f, 0.0f) {
    SetPosition({0.0f, 0.0f}, false);

    // Compile shader
    _program = glCreateProgram();
    const GLchar program_name[] = "Canvas Program";
    glObjectLabel(GL_PROGRAM, _program, sizeof(program_name), program_name);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const auto canvas_vert_file = _app->_data.open("shaders/Canvas.vert");
    std::string_view canvas_vert(canvas_vert_file.begin(), canvas_vert_file.end());
    const char *canvas_vert_source = canvas_vert.data();
    glShaderSource(vertex_shader, 1, &canvas_vert_source, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto canvas_frag_file = _app->_data.open("shaders/Canvas.frag");
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

    // Bind uniform
    GLuint matrices_index = glGetUniformBlockIndex(_program, "Matrices");
    glUniformBlockBinding(_program, matrices_index, 0);

    // Create texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    const GLchar texture_name[] = "Canvas Texture";
    glObjectLabel(GL_TEXTURE, _texture, sizeof(texture_name), texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const std::vector<std::uint32_t> pixels(_size.x * _size.y, 0xFFFFFFFF);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _size.x, _size.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _size.x, _size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Create vertex array
    glGenVertexArrays(1, &_mesh);
    assert(_mesh && "Failed to create canvas vertex array object");
    glBindVertexArray(_mesh);
    const GLchar mesh_name[] = "Canvas VertexArray";
    glObjectLabel(GL_VERTEX_ARRAY, _mesh, sizeof(mesh_name), mesh_name);

    UpdateModel();
}

Canvas::~Canvas() {
    glDeleteVertexArrays(1, &_mesh);
    glDeleteTextures(1, &_texture);
    glDeleteProgram(_program);
}

void Canvas::Render() const {
    glUseProgram(_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glUniformMatrix4fv(glGetUniformLocation(_program, "model"), 1, GL_FALSE, glm::value_ptr(_model));
    glBindVertexArray(_mesh);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Canvas::UpdateModel() {
    _model = glm::mat4(1.0f);
    _model = glm::scale(_model, glm::vec3(_size, 1.0f));
    _model = glm::translate(_model, glm::vec3(_position, 0.0f));
}

void Canvas::SetPosition(glm::vec2 position, bool update) {
    _position = position;
    if (update) {
        UpdateModel();
    }
}
