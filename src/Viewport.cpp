#include "Viewport.h"
#include "App.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Viewport::Viewport(const App *app) : _app(app) {
    SetPosition({0.0f, 0.0f}, false);
    SetZoom(0.0f, false);
    SetRotation(0.0f, false);
    UpdateView();
}

Viewport::~Viewport() {
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

void Viewport::UpdateView() {
    _viewport = glm::mat4(1.0f);
    _viewport = glm::translate(_viewport, glm::vec3(_position, 0.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, _app->_ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(_viewport));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
