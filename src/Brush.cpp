#include "Brush.h"

#include "App.h"
#include "Canvas.h"
#include "Tile.h"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

Brush::Brush(const App *app) : _app(app), _brush_parameters() {
    _program_work_group_size = {0, 0, 0};
    _program = glCreateProgram();
    const GLchar program_name[] = "Brush Program";
    glObjectLabel(GL_PROGRAM, _program, sizeof(program_name), program_name);

    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    const auto canvas_comp_file = _app->_data.open("shaders/Brush.comp");
    std::string_view canvas_comp(canvas_comp_file.begin(), canvas_comp_file.end());
    const char *canvas_comp_source = canvas_comp.data();
    glShaderSource(compute_shader, 1, &canvas_comp_source, nullptr);
    glCompileShader(compute_shader);

    glAttachShader(_program, compute_shader);
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

    glDeleteShader(compute_shader);

    GLint work_group_size[3]{};
    glGetProgramiv(_program, GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    _program_work_group_size.x = work_group_size[0];
    _program_work_group_size.y = work_group_size[1];
    _program_work_group_size.z = work_group_size[2];

    glGenBuffers(1, &_ubo_brush_parameters);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
    const GLchar ubo_matrices_name[] = "Brush parameters Uniform Buffer";
    glObjectLabel(GL_BUFFER, _ubo_brush_parameters, sizeof(ubo_matrices_name), ubo_matrices_name);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(BrushParameters), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // TODO: Change the binding point to a constant that can be accesed by other program
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, _ubo_brush_parameters, 0, sizeof(BrushParameters));

    _brush_parameters._pressure = 1.0f;
    _brush_parameters._tilt = {0.0f, 0.0f};
    _brush_parameters._position = {0.0f, 0.0f};
    _brush_parameters._color = {0.0f, 0.0f, 0.0f, 1.0f};
    _brush_parameters._radius = 10.0f;
    _brush_parameters._hardness = 0.9f;

    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(BrushParameters), &_brush_parameters);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint i = glGetUniformBlockIndex(_program, "brush_parameters");
}

Brush::~Brush() {
    glDeleteProgram(_program);
    _program = 0;
}

void Brush::Use(const Tile *tile) const {
    if (_brush_parameters._color.a <= 0.0f)
        return;
    if (_brush_parameters._radius <= 0.0f)
        return;

    glUseProgram(_program);

    tile->BindUniform();

    glBindImageTexture(0, tile->TextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(tile->Data()._size.x / _program_work_group_size.x,
                      tile->Data()._size.y / _program_work_group_size.y, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, tile->TextureID());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}

void Brush::SetPosition(glm::vec2 position, bool update) {
    _brush_parameters._position = position;
    if (update) {
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(BrushParameters, _position), sizeof(BrushParameters::_position),
                        &_brush_parameters._position);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Brush::SetColor(glm::vec4 color, bool update) {
    _brush_parameters._color = color;
    if (update) {
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(BrushParameters, _color), sizeof(BrushParameters::_color),
                        &_brush_parameters._color);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Brush::SetHardness(float hardness, bool update) {
    spdlog::info("Set brush hardness to: {:.2f}", hardness);
    _brush_parameters._hardness = hardness;
    if (update) {
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(BrushParameters), &_brush_parameters);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Brush::SetRadius(float radius, bool update) {
    spdlog::info("Set brush radius to: {:.2f}px", radius);
    _brush_parameters._radius = _radius * _radius_factor;
    if (update) {
        glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(BrushParameters), &_brush_parameters);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Brush::SetRadiusFactor(float factor) {
    _radius_factor = factor;
    _brush_parameters._radius = _radius * _radius_factor;
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_brush_parameters);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(BrushParameters), &_brush_parameters);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

float Brush::GetRadius() const {
    return _brush_parameters._radius / _radius_factor;
}

float Brush::GetHardness() const {
    return _brush_parameters._hardness;
}
