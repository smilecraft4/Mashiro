#include "pch.h"

#include "Viewport.h"

Viewport::Viewport(glm::ivec2 size) {
    _buffer = Uniformbuffer::Create(TEXT("Viewport Matrices"), 0, sizeof(Matrices), nullptr);

    SetSize(size);
    SetPosition({0.0f, 0.0f}, false);
    SetZoom(1.0f, false);
    SetRotation(0.0f, false);
    UpdateViewMatrix();
}

Viewport::~Viewport() {
}

void Viewport::SetSize(glm::ivec2 size) {
    _size = size;
    UpdateProjMatrix();
}

void Viewport::SetPosition(glm::vec2 position, bool update) {
    _position = position;

    if (update) {
        UpdateViewMatrix();
    }
}

void Viewport::SetRotation(float rotation, bool update) {
    _rotation = rotation;
    if (update) {
        UpdateViewMatrix();
    }
}

void Viewport::SetZoom(float zoom, bool update) {
    _zoom = zoom;
    if (update) {
        UpdateViewMatrix();
    }
}

glm::ivec2 Viewport::GetSize() const noexcept {
    return _size;
}

glm::vec2 Viewport::GetPosition() const noexcept {
    return _position;
}

float Viewport::GetRotation() const noexcept {
    return _rotation;
}

float Viewport::GetZoom() const noexcept {
    return _zoom;
}

bool Viewport::IsVisible(AABB other) {
    return AABB::Overlap(_aabb, other);
}

void Viewport::UpdateViewMatrix() {
    const auto translation = glm::translate(glm::mat4(1.0f), glm::vec3(_position, 0.0f));
    const auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    const auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(_zoom));

    _matrices.view = translation * scale * rotation;
    _buffer->SetData(offsetof(Matrices, view), sizeof(Matrices::view), &_matrices.view);

    _aabb = {{0.0f, 0.0f}, {1.0f, 1.0f}};
}

void Viewport::UpdateProjMatrix() {
    _matrices.proj = glm::ortho(-std::floor(_size.x / 2.0f), std::ceil(_size.x / 2.0f), -std::floor(_size.y / 2.0f),
                                std::ceil(_size.y / 2.0f));
    _buffer->SetData(offsetof(Matrices, proj), sizeof(Matrices::proj), &_matrices.proj);

    _aabb = {{0.0f, 0.0f}, {1.0f, 1.0f}};
}
