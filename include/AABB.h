#pragma once

#include <glm/vec2.hpp>

struct AABB {
    glm::vec2 min;
    glm::vec2 max;

    static bool Overlap(AABB &a, AABB &b) noexcept {
        // TODO
        return true;
    }
};
