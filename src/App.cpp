#include "App.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <stb_image.h>

CMRC_DECLARE(mashiro);

App::App() : _data(cmrc::mashiro::get_filesystem()), _painting{}, _navigation{} {

    HRESULT hr = CoInitialize(NULL);
    // TODO: load from last openned LoadLastFilePreferences();
    _width = 800;
    _height = 600;
    _title = "Mashiro";
    _fullscreen = false;

    _settings = std::make_unique<Settings>();

    InitGLFW();
    InitOpenGL();

    _screen_program = glCreateProgram();
    std::string screen_program_name = "Screen Program";
    glObjectLabel(GL_PROGRAM, _screen_program, screen_program_name.size(), screen_program_name.c_str());

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const auto canvas_vert_file = _data.open("shaders/screen.vert");
    std::string_view canvas_vert(canvas_vert_file.begin(), canvas_vert_file.end());
    const char *canvas_vert_source = canvas_vert.data();
    glShaderSource(vertex_shader, 1, &canvas_vert_source, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto canvas_frag_file = _data.open("shaders/screen.frag");
    std::string_view canvas_frag(canvas_frag_file.begin(), canvas_frag_file.end());
    const char *canvas_frag_source = canvas_frag.data();
    glShaderSource(fragment_shader, 1, &canvas_frag_source, nullptr);
    glCompileShader(fragment_shader);

    glAttachShader(_screen_program, vertex_shader);
    glAttachShader(_screen_program, fragment_shader);
    glLinkProgram(_screen_program);

    GLint isLinked = 0;
    glGetProgramiv(_screen_program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(_screen_program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(_screen_program, maxLength, &maxLength, &infoLog[0]);
        spdlog::critical("{}", infoLog.data());
    }
    assert(isLinked && "Fail to linked program");

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGenVertexArrays(1, &_screen_mesh);
    assert(_screen_mesh && "Failed to create canvas vertex array object");
    glBindVertexArray(_screen_mesh);
    std::string screen_mesh_name = "Screen Mesh";
    glObjectLabel(GL_VERTEX_ARRAY, _screen_mesh, screen_mesh_name.size(), screen_mesh_name.c_str());


    _canvas = std::make_unique<Canvas>(this, glm::ivec2(_settings->_tile_resolution, _settings->_tile_resolution));

    auto file = File::Find("./");
    if (file.has_value()) {
        std::swap(_file, file.value());
        _settings->_recent_files.push_back(_file->GetFilename());
    } else {
        _file = std::make_unique<File>(_settings->_tile_resolution);
        _file->SetFilename("./mashiro.msh");
        _settings->_recent_files.push_back("./mashiro.msh");
    }

    _canvas->LoadFile();

    glfwSetWindowTitle(_window, _file->GetFilename().string().c_str());

    _viewport = std::make_unique<Viewport>(this, _width, _height);
    _brush = std::make_unique<Brush>(this);

    _brush->SetColor({0.0f, 255.0f, 0.0f, 255.0f});
    _brush->SetPosition({0, 0});

    _stylus = std::make_unique<Stylus>(this);
}

App::~App() {
    _canvas->SaveTiles();

    glfwDestroyWindow(_window);
    glfwTerminate();
    CoUninitialize();
}

void App::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwWaitEvents();
        Update();
        Render();
    }
}

void App::Update() {
    Paint();
    Navigate();
}

void App::Render() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(_screen_program);
    glBindVertexArray(_screen_mesh);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);

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
            app->_navigation.enabled = true;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_SHIFT:
            app->_navigation.rotating = true;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_LEFT_CONTROL:
            app->_navigation.zooming = true;
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
        // spdlog::info("enabled {}\t rotating {}\t zooming {}", app->_navigation.enabled, app->_navigation.rotating,
        //             app->_navigation.zooming);
    }

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_SPACE:
            app->_navigation.enabled = false;
            break;
        case GLFW_KEY_RIGHT_SHIFT:
        case GLFW_KEY_LEFT_SHIFT:
            app->_navigation.rotating = false;
            break;
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_LEFT_CONTROL:
            app->_navigation.zooming = false;
            break;
        default:
            break;
        }
        // spdlog::info("enabled {}\t rotating {}\t zooming {}", app->_navigation.enabled, app->_navigation.rotating,
        //              app->_navigation.zooming);
    }

    app->Update();
    app->Render();
}

void App::CursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    app->Update();
    app->Render();
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

    app->Update();
    app->Render();
}

void App::ButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (app->_navigation.enabled) {
            app->_viewport->SetPivot(app->_navigation.cursor_current);
            app->_navigation.using_hand = (action != GLFW_RELEASE);
        } else if (action == GLFW_PRESS && !app->_painting.enabled) {
            app->_brush->SetColor({0.0f, 0.0f, 0.0f, 255.0f});
            app->_painting.enabled = true;
        } else if (action == GLFW_RELEASE) {
            app->_painting.enabled = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS && !app->_painting.enabled) {
            app->_brush->SetColor({255.0f, 255.0f, 255.0f, 255.0f});
            app->_painting.enabled = true;
        } else if (action == GLFW_RELEASE) {
            app->_painting.enabled = false;
        }
    }

    app->Update();
    app->Render();
}

void App::FramebufferSize(GLFWwindow *window, int width, int height) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    app->_width = width;
    app->_height = height;
    glViewport(0, 0, width, height);

    app->_viewport->SetSize({width, height});

    app->Update();
    app->Render();
}

void App::Paint() {
    if (!_painting.enabled) {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(_window, &xpos, &ypos);
    glm::vec4 a = glm::vec4(xpos, _height - ypos, 0.0f, 1.0f);
    a /= glm::vec4(_viewport->_size, 1.0f, 1.0f);
    a *= glm::vec4(2.0f, 2.0f, 0.0f, 1.0f);
    a += glm::vec4(-1.0f, -1.0f, 0.0f, 0.0f);
    a = glm::inverse(_viewport->_matrices._proj) * a;
    a = glm::inverse(_viewport->_matrices._view) * a;

    std::vector<glm::vec2> brush_path{{a.x, a.y}};
    const glm::ivec2 coord = glm::floor(brush_path[0] / glm::vec2(_canvas->_tiles_size));
    // spdlog::info("({},{}) -> ({}, {}) -> ({}, {})", xpos, ypos, a.x, a.y, coord.x, coord.y);
    _canvas->UpdateTilesProcessed(brush_path, _brush->GetRadius());
    _brush->SetPosition(a);
    _canvas->Process(_brush.get());
}

void App::Navigate() {
    if (!_navigation.enabled) {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(_window, &xpos, &ypos);
    _navigation.cursor_previous = _navigation.cursor_current;
    _navigation.cursor_current = {xpos, ypos};
    _navigation.delta_mouse = _navigation.cursor_current - _navigation.cursor_previous;
    _navigation.delta_mouse.y = -_navigation.delta_mouse.y;

    if (_navigation.using_hand) {
        if (_navigation.rotating) {
            auto rot = _viewport->GetRotation();
            rot += (_navigation.delta_mouse.x + _navigation.delta_mouse.y) / 10.0f;

            rot = fmod(rot, 360);
            if (rot < 0)
                rot += 360;

            _viewport->SetRotation(rot);
        } else if (_navigation.zooming) {
            auto zoom = _viewport->GetZoom();
            zoom = std::max(
                0.05f,
                std::min(1000.0f, zoom + (float)(_navigation.delta_mouse.x + _navigation.delta_mouse.y) / 666.0f));
            _viewport->SetZoom(zoom);
        } else {
            // TODO: add drifting at the end, like launching a piece of paper and catching it
            auto zoom = _viewport->GetZoom();
            auto pos = _viewport->GetPosition();
            pos += _navigation.delta_mouse / zoom;
            _viewport->SetPosition(pos);
        }
    }

    // calculate settings
}

void static OpenglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam);
void static GlfwErrorCallback(int error_code, const char *description);

bool App::InitGLFW() {
    if (!glfwInit()) {
        return false;
    }

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

    double x, y;
    glfwGetCursorPos(_window, &x, &y);
    _navigation.cursor_current.x = x;
    _navigation.cursor_current.y = y;
    _navigation.cursor_previous.x = x;
    _navigation.cursor_previous.y = y;

    glfwSetFramebufferSizeCallback(_window, FramebufferSize);
    glfwSetMouseButtonCallback(_window, ButtonCallback);
    glfwSetScrollCallback(_window, ScrollCallback);
    glfwSetCursorPosCallback(_window, CursorPosCallback);
    glfwSetKeyCallback(_window, KeyCallback);

    return true;
}

bool App::InitOpenGL() {
    glfwMakeContextCurrent(_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSwapInterval(0);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenglErrorCallback, nullptr);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glDepthRange(0.0f, 1.0f);

    return true;
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