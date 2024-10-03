#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

const glm::vec3 canvas_up = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 canvas_right = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 canvas_forward = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 canvas_down = -canvas_up;
const glm::vec3 canvas_left = -canvas_right;
const glm::vec3 canvas_backward = -canvas_backward;

const glm::vec2 view_up = glm::vec2(1.0f, 0.0f);
const glm::vec2 view_right = glm::vec2(0.0f, 1.0f);
const glm::vec2 view_down = -view_up;
const glm::vec2 view_left = -view_right;
