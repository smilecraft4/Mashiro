#include "Mashiro.h"

#include <stdexcept>

#include <glad/glad.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog/spdlog.h>

namespace GLFW {} // namespace GLFW

Mashiro::Mashiro() {
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
}

void Mashiro::InitWindow() {
    InitCursors();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

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

// TODO: Create custom simple cursors
void Mashiro::InitCursors() {
    _cursor_select = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    _cursor_paint = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    _cursor_erase = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    _cursor_pan = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    _cursor_zoom = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    _cursor_rotate = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
}

void Mashiro::SetCursorState(State new_state) {
    _cursor_state = new_state;

    switch (new_state) {
    case Mashiro::Painting:
        glfwSetCursor(_window, _cursor_paint);
        break;
    case Mashiro::Erasing:
        glfwSetCursor(_window, _cursor_erase);
        break;
    case Mashiro::Selecting:
        glfwSetCursor(_window, _cursor_select);
        break;
    case Mashiro::Panning:
        glfwSetCursor(_window, _cursor_pan);
        break;
    case Mashiro::Zooming:
        glfwSetCursor(_window, _cursor_zoom);
        break;
    case Mashiro::Rotating:
        glfwSetCursor(_window, _cursor_rotate);
        break;
    default:
        glfwSetCursor(_window, _cursor_select);
        break;
    }
}

Mashiro::~Mashiro() {
    glfwDestroyWindow(_window);

    glfwDestroyCursor(_cursor_select);
    glfwDestroyCursor(_cursor_paint);
    glfwDestroyCursor(_cursor_erase);
    glfwDestroyCursor(_cursor_pan);
    glfwDestroyCursor(_cursor_rotate);
    glfwDestroyCursor(_cursor_zoom);

    glfwTerminate();
}

void Mashiro::Run() {
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
    }
}

void Mashiro::OnRender() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(_window);
}

void Mashiro::OnResize(int width, int height) {
    _width = width;
    _height = height;

    glViewport(0, 0, width, height);

    // TODO: resize all framebuffers here
}

#pragma region GLFW Callbacks
static Mashiro *GetGlfwWindowUserPointer(GLFWwindow *window) {
    auto pointer = glfwGetWindowUserPointer(window);
    if (!pointer) {
        throw std::runtime_error("Missing window user pointer");
    }
    return static_cast<Mashiro *>(pointer);
}

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
    _lastTime = _currentTime;
    _currentTime = glfwGetTime();
    _elapsedTime = _currentTime - _lastTime;
}

void Mashiro::ToggleFullscreen() {
    if (_fullscreen) {
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowMonitor(_window, NULL, _windowed_x, _windowed_y, _windowed_width, _windowed_height,
                             GLFW_DONT_CARE);
        _fullscreen = false;
    } else {
        glfwGetWindowPos(_window, &_windowed_x, &_windowed_y);
        glfwGetWindowSize(_window, &_windowed_width, &_windowed_height);

        // TODO: get the current window monitor
        // https://stackoverflow.com/a/31526753
        auto monitor = glfwGetPrimaryMonitor();
        if (!monitor) {
            spdlog::error("Failed to find monitor");
            return;
        }
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowMonitor(_window, NULL, 0, 0, mode->width, mode->height, 0);
        _fullscreen = true;
    }
}

void Mashiro::GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto mashiro = GetGlfwWindowUserPointer(window);

    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        mashiro->ToggleFullscreen();
    }

    // FIXME: Fix bug when exiting and re-entering window (the cursor does not keep the correct state)
    if (key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) {
            mashiro->SetCursorState(State::Panning);
        } else if (action == GLFW_RELEASE) {
            mashiro->SetCursorState(State::Painting);
        }
    }
}

void Mashiro::GLFWCharCallback(GLFWwindow *window, unsigned int codepoint) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWCursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    auto mashiro = GetGlfwWindowUserPointer(window);
}

void Mashiro::GLFWCursorEnterCallback(GLFWwindow *window, int entered) {
    auto mashiro = GetGlfwWindowUserPointer(window);
    if (entered) {
        mashiro->SetCursorState(mashiro->_cursor_state);
    } else {
        glfwSetCursor(window, NULL);
    }
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
