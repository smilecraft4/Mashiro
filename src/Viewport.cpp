#include "Viewport.h"
#include "App.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
// #include <glm/gtx/rotate_vector.hpp>

Viewport::Viewport(const App *app, int width, int height) : _app(app) {
    glGenBuffers(1, &_matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _matrices_ubo);
    const GLchar ubo_matrices_name[] = "Matrices Uniform Buffer";
    glObjectLabel(GL_BUFFER, _matrices_ubo, sizeof(ubo_matrices_name), ubo_matrices_name);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _matrices_ubo, 0, sizeof(Matrices));


    SetPosition({0.0f, 0.0f}, false);
    SetZoom(1.0f, false);
    SetRotation(0.0f, false);
    SetPivot({0.0f, 0.0f});
    UpdateView();

    SetSize({width, height});
}

Viewport::~Viewport() {
    glDeleteBuffers(1, &_matrices_ubo);
}

void Viewport::UpdateView() {
    spdlog::info("position: {:.1f}{:.1f}, rotation {:.1f}, scale {:.1f}", _position.x, _position.y, _rotation, _zoom);

    const auto rotate = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    const auto unrotate = glm::rotate(glm::mat4(1.0f), glm::radians(-_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    const auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(_zoom, _zoom, 1.0f));
    const glm::vec3 t = unrotate * glm::vec4(_position, 0.0f, 1.0f);

    // the center of the viewport (_position) is the center of local space
    // the viewport has a rotation and a scale all of them happen at _position
    _matrices._view = rotate * scale * glm::translate(glm::mat4(1.0f), glm::vec3(t));

    // the center of the canvas (0,0) is the center of the global space

    // execute rotation, scale, in local space
    // execute translation in global space
    // the viewport is the local space
    // the canvas is global space

    //_matrices._view = glm::translate(_matrices._view, glm::vec3(_position, 0.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, _matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Matrices, _view), sizeof(Matrices::_view),
                    glm::value_ptr(_matrices._view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // TODO: CalculateAABB();
}

void Viewport::UpdateProj() {
    _matrices._proj = glm::orthoLH(-std::floorf(_size.x / 2.0f), std::floorf(_size.x / 2.0f),
                                   -std::floorf(_size.y / 2.0f), std::floorf(_size.y / 2.0f), 0.0f, 1.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, _matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Matrices, _proj), sizeof(Matrices::_proj),
                    glm::value_ptr(_matrices._proj));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Viewport::CalculateAABB() {
    // get's the largest AABB encomposing the viewport
}

void Viewport::SetPivot(glm::vec2 pivot) {
    _pivot = pivot;
}

void Viewport::SetSize(glm::ivec2 size, bool update) {
    _size = size;
    if (update) {
        UpdateProj();
    }
}

void Viewport::SetPosition(glm::vec2 position, bool update) {
    _position = position;
    if (update) {
        UpdateView();
    }
}

void Viewport::SetZoom(float zoom, bool update) {
    _zoom = zoom;
    if (update) {
        UpdateView();
    }
}

void Viewport::SetRotation(float rotation, bool update) {
    _rotation = rotation;
    if (update) {
        UpdateView();
    }
}

glm::vec2 Viewport::GetPosition() const {
    return _position;
}

glm::ivec2 Viewport::GetSize() const {
    return _size;
}

float Viewport::GetZoom() const {
    return _zoom;
}

float Viewport::GetRotation() const {
    return _rotation;
}
