#include "Mashiro.h"

#include <stdexcept>

#include <glad/glad.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog/spdlog.h>

namespace GLFW {

} // namespace GLFW

Mashiro::Mashiro() {
    _window = NULL;
    _width = 800;
    _height = 600;
    _title = "Mashiro";
    _elapsed_time = 0.0;
    _last_time = 0.0;
    _current_time = 0.0;

    InitConsole();
    InitGLFW();
    InitWindow();
    InitOpenGL();
}

void Mashiro::InitOpenGL() {
    glfwMakeContextCurrent(_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize glad");
    }
}

void Mashiro::InitConsole() {
    SetConsoleOutputCP(CP_UTF8);
}

void Mashiro::InitGLFW() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwSetErrorCallback(GLFWErrorCallback);
    glfwSetErrorCallback(GLFWErrorCallback);
    glfwSetMonitorCallback(GLFWMonitorCallback);
    glfwSetJoystickCallback(GLFWJoystickCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void Mashiro::InitWindow() {
    _window = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
    if (!_window) {
        throw std::runtime_error("Failed to create window");
    }

    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, GLFWKeyCallback);
    glfwSetCharCallback(_window, GLFWCharCallback);
    glfwSetDropCallback(_window, GLFWDropCallback);
    glfwSetScrollCallback(_window, GLFWScrollCallback);
    glfwSetCursorPosCallback(_window, GLFWCursorPosCallback);
    glfwSetWindowPosCallback(_window, GLFWWindowPosCallback);
    glfwSetWindowSizeCallback(_window, GLFWWindowSizeCallback);
    glfwSetCursorEnterCallback(_window, GLFWCursorEnterCallback);
    glfwSetMouseButtonCallback(_window, GLFWMouseButtonCallback);
    glfwSetWindowContentScaleCallback(_window, GLFWWindowContentScaleCallback);
    glfwSetWindowCloseCallback(_window, GLFWWindowCloseCallback);
    glfwSetWindowFocusCallback(_window, GLFWWindowFocusCallback);
    glfwSetWindowIconifyCallback(_window, GLFWWindowIconifyCallback);
    glfwSetWindowRefreshCallback(_window, GLFWWindowRefreshCallback);
    glfwSetWindowMaximizeCallback(_window, GLFWWindowMaximizeCallback);
    glfwSetFramebufferSizeCallback(_window, GLFWFramebufferSizeCallback);
}

Mashiro::~Mashiro() {
}

void Mashiro::Run() {
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
    }
}

void Mashiro::OnRender() {    
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(_window);
}

void Mashiro::OnResize(int width, int height) {
    _width = width;
    _height = height;

    glViewport(0, 0, width, height);

    // TODO: resize all framebuffers here
}

static Mashiro *GetGlfwWindowUserPointer(GLFWwindow *window) {
    auto pointer = glfwGetWindowUserPointer(window);
    if (!pointer) {
        throw std::runtime_error("Missing window user pointer");
    }
    return static_cast<Mashiro *>(pointer);
}

#pragma region GLFW Callbacks
void Mashiro::GLFWErrorCallback(int error, const char *description) {
    spdlog::error("GLFW: {} {}", error, description);
}

void Mashiro::GLFWMonitorCallback(GLFWmonitor *monitor, int event) {
}

void Mashiro::GLFWWindowCloseCallback(GLFWwindow *window) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowSizeCallback(GLFWwindow *window, int width, int height) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWFramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    auto mashiro = GetGlfwWindowUserPointer(window);
    mashiro->OnResize(width, height);
}

void Mashiro::GLFWWindowContentScaleCallback(GLFWwindow *window, float xscale, float yscale) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowPosCallback(GLFWwindow *window, int xpos, int ypos) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowIconifyCallback(GLFWwindow *window, int iconified) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowMaximizeCallback(GLFWwindow *window, int maximized) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowFocusCallback(GLFWwindow *window, int focused) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWWindowRefreshCallback(GLFWwindow *window) {
    auto mashiro = GetGlfwWindowUserPointer(window);

    mashiro->UpdateElapsedTime();
    mashiro->OnRender();
}

void Mashiro::UpdateElapsedTime() {
    _last_time = _current_time;
    _current_time = glfwGetTime();
    _elapsed_time = _current_time - _last_time;
}

void Mashiro::GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWCharCallback(GLFWwindow *window, unsigned int codepoint) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWCursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWCursorEnterCallback(GLFWwindow *window, int entered) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWJoystickCallback(int jid, int event) {
}

void Mashiro::GLFWDropCallback(GLFWwindow *window, int count, const char **paths) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}
#pragma endregion
