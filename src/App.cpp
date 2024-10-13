#include "App.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <stb_image.h>

CMRC_DECLARE(mashiro);

void static OpenglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam);
void static GlfwErrorCallback(int error_code, const char *description);

App::App() : _data(cmrc::mashiro::get_filesystem()) {

    // TODO: load from last openned LoadLastFilePreferences();
    _width = 800;
    _height = 600;
    _title = "Mashiro";
    _painting = false;
    _fullscreen = false;
    _zooming = false;
    _rotating = false;
    _panning = false;
    _using_hand = false;

    assert(glfwInit() && "Failed to initialize GLFW");

    glfwSetErrorCallback(GlfwErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    _window = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
    assert(_window && "Failed to create window");
    glfwSetWindowUserPointer(_window, this);

    GLFWimage icons[3]{};
    {
        const auto icon_16_file = _data.open("images/icon_16x16.png");
        std::vector<unsigned char> icon_16(icon_16_file.begin(), icon_16_file.end());
        icons[0].pixels = stbi_load_from_memory(icon_16.data(), icon_16.size(), &icons[0].width, &icons[0].height,
                                                nullptr, STBI_rgb_alpha);

        const auto icon_32_file = _data.open("images/icon_32x32.png");
        std::vector<unsigned char> icon_32(icon_32_file.begin(), icon_32_file.end());
        icons[1].pixels = stbi_load_from_memory(icon_32.data(), icon_32.size(), &icons[1].width, &icons[1].height,
                                                nullptr, STBI_rgb_alpha);

        const auto icon_48_file = _data.open("images/icon_48x48.png");
        std::vector<unsigned char> icon_48(icon_48_file.begin(), icon_48_file.end());
        icons[2].pixels = stbi_load_from_memory(icon_48.data(), icon_48.size(), &icons[2].width, &icons[2].height,
                                                nullptr, STBI_rgb_alpha);
    }

    glfwSetWindowIcon(_window, 3, icons);

    glfwMakeContextCurrent(_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(0);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenglErrorCallback, nullptr);

    // Set Opengl Render state

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glDepthRange(0.0f, 1.0f);

    glfwSetFramebufferSizeCallback(_window, FramebufferSize);
    glfwSetMouseButtonCallback(_window, ButtonCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    glfwSetCursorPosCallback(_window, CursorPosCallback);
    glfwSetKeyCallback(_window, KeyCallback);

    glGenBuffers(1, &_ubo_matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_matrices);
    const GLchar ubo_matrices_name[] = "Matrices Uniform Buffer";
    glObjectLabel(GL_BUFFER, _ubo_matrices, sizeof(ubo_matrices_name), ubo_matrices_name);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // TODO: Change the binding point to a constant that can be accesed by other program
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, _ubo_matrices, 0, sizeof(glm::mat4) * 2);

    glViewport(0, 0, _width, _height);
    _projection = glm::orthoLH(-_width / 2.0f, _width / 2.0f, -_height / 2.0f, _height / 2.0f, 0.0f, 1.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(_projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    _canvas = std::make_unique<Canvas>(this);
    _viewport = std::make_unique<Viewport>(this);
    _brush = std::make_unique<Brush>(this);

    _brush->SetColor({0.0f, 255.0f, 0.0f, 255.0f});
    _brush->SetPosition({0, 0});
}

App::~App() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void App::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwWaitEvents();
        Update();
        Render();
    }
}

void App::Update() {
}

void App::Render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _canvas->Render();

    glfwSwapBuffers(_window);
}

void App::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    // TODO: Add a shortcut handler
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        app->ToggleFullscreen();
    }

    // Hand Tool (Pan, Zoom, Rotate)
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_SPACE:
            app->_panning = true;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_SHIFT:
            app->_rotating = true;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_LEFT_CONTROL:
            app->_zooming = true;
            break;
        case GLFW_KEY_S:
            if (mods & GLFW_MOD_ALT) {
                app->_viewport->SetZoom(1.0f);
            }
            break;
        case GLFW_KEY_R:
            if (mods & GLFW_MOD_ALT) {
                app->_viewport->SetRotation(0.0f);
            }
            break;
        case GLFW_KEY_G:
            if (mods & GLFW_MOD_ALT) {
                app->_viewport->SetPosition({0.0f, 0.0f});
            }
            break;
        default:
            break;
        }
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_SPACE:
            app->_panning = false;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_SHIFT:
            app->_rotating = false;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_LEFT_CONTROL:
            app->_zooming = false;
            break;
        default:
            break;
        }
    }
}

void App::CursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    glm::dvec2 delta = (glm::dvec2(xpos, ypos) - app->_cursor_previous) * glm::dvec2(1.0, -1.0);

    if (app->_panning) {
        if (app->_rotating) {
            auto rot = app->_viewport->GetRotation();
            rot += (delta.x + delta.y) / 10.0f;

            { // Wrap angle [0;360[
                rot = fmod(rot, 360);
                if (rot < 0)
                    rot += 360;
            }

            app->_viewport->SetRotation(rot);
        } else if (app->_zooming) {
            auto zoom = app->_viewport->GetZoom();
            zoom = std::max(0.05f, std::min(1000.0f, zoom + (float)(delta.x + delta.y) / 666.0f));
            app->_viewport->SetZoom(zoom);
        } else {
            // TODO: add drifting at the end, like launching a piece of paper and catching it
            auto zoom = app->_viewport->GetZoom();
            auto pos = app->_viewport->GetPosition();
            pos += glm::vec2(delta);
            app->_viewport->SetPosition(pos);
        }
    }

    if (app->_painting) {
        // convert cursor pos to screen_space [(pos_x, pos_y); (pos_x + width, pos_y + height)] --> [(-1,-1); (1,1)]
        glm::vec4 brush_pos = {xpos / (float)app->_width * 2.0f - 1.0f,
                               (ypos / (float)app->_height * 2.0f - 1.0f) * -1.0f, 0.0f, 0.0f};
        auto temp = glm::inverse(app->_viewport->_viewport) * glm::vec4(app->_viewport->GetPosition(), 0.0f, 0.0f);
        brush_pos = (glm::inverse(app->_viewport->_viewport) * glm::inverse(app->_projection) * brush_pos) - temp;
        app->_brush->SetPosition(brush_pos);
        app->_brush->Use(app->_canvas.get());
    }

    app->_cursor_previous = {xpos, ypos};
}

void App::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    int state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) | glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    if (state == GLFW_PRESS) {
        const auto hardness = app->_brush->GetHardness() + (float)yoffset * 0.01f;
        app->_brush->SetHardness(std::max(0.0f, std::min(1.0f, hardness)));
    } else {
        const auto radius = app->_brush->GetRadius() + (float)yoffset * 0.5f;
        app->_brush->SetRadius(std::max(0.0f, std::min(500.0f, radius)));
    }
}

void App::ButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (app->_panning) {
            app->_using_hand = (action != GLFW_RELEASE);
        } else if (action == GLFW_PRESS && !app->_painting) {
            app->_brush->SetColor({0.0f, 0.0f, 0.0f, 255.0f});
            app->_painting = true;

            double xpos, ypos;
            glfwGetCursorPos(app->_window, &xpos, &ypos);
            glm::vec4 brush_pos = {xpos / (float)app->_width * 2.0f - 1.0f,
                                   (ypos / (float)app->_height * 2.0f - 1.0f) * -1.0f, 0.0f, 0.0f};
            auto temp = glm::inverse(app->_viewport->_viewport) * glm::vec4(app->_viewport->GetPosition(), 0.0f, 0.0f);
            brush_pos = (glm::inverse(app->_viewport->_viewport) * glm::inverse(app->_projection) * brush_pos) - temp;
            app->_brush->SetPosition(brush_pos);

            app->_brush->Use(app->_canvas.get());
        } else if (action == GLFW_RELEASE) {
            app->_painting = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS && !app->_painting) {
            app->_brush->SetColor({255.0f, 255.0f, 255.0f, 127.0f});
            app->_painting = true;

            double xpos, ypos;
            glfwGetCursorPos(app->_window, &xpos, &ypos);
            glm::vec4 brush_pos = {xpos / (float)app->_width * 2.0f - 1.0f,
                                   (ypos / (float)app->_height * 2.0f - 1.0f) * -1.0f, 0.0f, 0.0f};
            auto temp = glm::inverse(app->_viewport->_viewport) * glm::vec4(app->_viewport->GetPosition(), 0.0f, 0.0f);
            brush_pos = (glm::inverse(app->_viewport->_viewport) * glm::inverse(app->_projection) * brush_pos) - temp;
            app->_brush->SetPosition(brush_pos);

            app->_brush->Use(app->_canvas.get());
        } else if (action == GLFW_RELEASE) {
            app->_painting = false;
        }
    }
}

void App::FramebufferSize(GLFWwindow *window, int width, int height) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    app->_width = width;
    app->_height = height;
    // TODO: BUG: when width or height are not whole number the canvas becomes blurry
    app->_projection = glm::orthoLH(-std::ceilf(app->_width / 2.0f), std::floorf(app->_width / 2.0f),
                                    -std::ceilf(app->_height / 2.0f), std::floorf(app->_height / 2.0f), 0.0f, 1.0f);
    glViewport(0, 0, app->_width, app->_height);
    glBindBuffer(GL_UNIFORM_BUFFER, app->_ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(app->_projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    app->Render();
}

void App::ToggleFullscreen() {
    // TODO: BUG: when in fullscreen and clicking the window blink for a one frame
    // TODO: BUG: There is still artefacting when toggle on fullscreen
    if (!_fullscreen) {
        // Save window size
        _saved_window_size.maximized = glfwGetWindowAttrib(_window, GLFW_MAXIMIZED);
        glfwGetWindowPos(_window, &_saved_window_size.xpos, &_saved_window_size.ypos);
        glfwGetWindowSize(_window, &_saved_window_size.width, &_saved_window_size.height);

        // Choose monitor
        int monitor_count;
        GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
        int monitor_x{}, monitor_y{}, monitor_width{}, monitor_height{};
        for (size_t i = 0; i < monitor_count; i++) {
            const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
            monitor_height = mode->height;
            monitor_width = mode->width;
            glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

            const int center_x = _saved_window_size.xpos + _saved_window_size.width / 2;
            const int center_y = _saved_window_size.ypos + _saved_window_size.height / 2;

            if (monitor_x <= center_x && center_x <= monitor_x + monitor_width && monitor_y <= center_y &&
                center_y <= monitor_y + monitor_height) {
                _monitor = monitors[i];
                break;
            }
        }

        glfwSetWindowAttrib(_window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowMonitor(_window, nullptr, monitor_x, monitor_y, monitor_width, monitor_height, 0);

        _fullscreen = true;
    } else {
        // Disable fullscreen option
        glfwSetWindowAttrib(_window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowMonitor(_window, nullptr, _saved_window_size.xpos, _saved_window_size.ypos,
                             _saved_window_size.width, _saved_window_size.height, 0);

        // Restore from _saved_window_size;
        if (_saved_window_size.maximized) {
            glfwMaximizeWindow(_window);
        }

        _monitor = nullptr;
        _fullscreen = false;
    }
}

void static OpenglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
    // glGetString()
    // TODO : convert this to a special logger
    spdlog::error("Opengl: {}, {}, {}, {}, {}", source, type, severity, id, message);
}

void static GlfwErrorCallback(int error_code, const char *description) {
    spdlog::error("GLFW: {}, {}", error_code, description);
}