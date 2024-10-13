#include "Viewport.h"
#include "App.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Viewport::Viewport(const App *app) : _app(app) {
    SetPosition({0.0f, 0.0f}, false);
    SetZoom(1.0f, false);
    SetRotation(0.0f, false);
    UpdateView();
}

Viewport::~Viewport() {
}

void Viewport::UpdateView() {
    _viewport = glm::mat4(1.0f);
    _viewport = glm::translate(_viewport, glm::vec3(_position, 0.0f));
    _viewport = glm::scale(_viewport, glm::vec3(_zoom));
    _viewport = glm::rotate(_viewport, glm::radians(_rotation), glm::vec3(0.0, 0.0f, 1.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, _app->_ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(_viewport));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    CalculateAABB();
}

void Viewport::CalculateAABB() {
    // get's the largest AABB encomposing the viewport 
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

float Viewport::GetZoom() const {
    return _zoom;
}

float Viewport::GetRotation() const {
    return _rotation;
}

glm::ivec4 Viewport::GetAABB() const {
    return _AABB;
}
