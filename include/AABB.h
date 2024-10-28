#pragma once

#include <glm/vec2.hpp>

struct AABB {
    glm::vec2 min;
    glm::vec2 max;

    static bool Overlap(AABB &a, AABB &b) noexcept {
        return (a.min.x <= b.max.x) && (a.max.x >= b.min.x) && (a.min.y <= b.max.y) && (a.max.y >= b.min.y);
    }
};
