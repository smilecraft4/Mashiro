#include "Mashiro.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

Mashiro::Mashiro() {
    InitConsole();
    InitGLFW();
    InitWindow();
    InitOpenGL();
    // InitImGui();

    InitCanvas();

    double x, y;
    glfwGetCursorPos(_window, &x, &y);
    _mouse_pos_previous.x = x;
    _mouse_pos_previous.y = y;
    _mouse_pos_current.x = x;
    _mouse_pos_current.y = y;

    UpdateCamera();
}

void Mashiro::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 16);
    io.Fonts->Build();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init();
}

void Mashiro::InitOpenGL() {
    glfwMakeContextCurrent(_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize glad");
    }

    // TODO: Setup opengl error callback
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
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

void Mashiro::InitCanvas() {
    // Create a quad
    const std::array<Vertex, 6> _canvas_vertices({
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{+1.0f, -1.0f}, {1.0f, 0.0f}},
        {{-1.0f, +1.0f}, {0.0f, 1.0f}},

        {{+1.0f, -1.0f}, {1.0f, 0.0f}},
        {{+1.0f, +1.0f}, {1.0f, 1.0f}},
        {{-1.0f, +1.0f}, {0.0f, 1.0f}},
    });

    // TODO: move this to mesh class
    glCreateBuffers(1, &_canvas_vbo);
    glNamedBufferStorage(_canvas_vbo, sizeof(Vertex) * _canvas_vertices.size(), _canvas_vertices.data(),
                         GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &_canvas_vao);

    glVertexArrayVertexBuffer(_canvas_vao, 0, _canvas_vbo, 0, sizeof(Vertex));

    // TODO: move this to Vertex struct
    glEnableVertexArrayAttrib(_canvas_vao, 0);
    glEnableVertexArrayAttrib(_canvas_vao, 1);
    glVertexArrayAttribFormat(_canvas_vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribFormat(_canvas_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex));
    glVertexArrayAttribBinding(_canvas_vao, 0, 0);
    glVertexArrayAttribBinding(_canvas_vao, 1, 0);

    // Create background texture

    const std::vector<std::uint32_t> _canvas_background_pixels(_canvas_width * _canvas_height, 0xFFFFFFFF);

    // TODO: change to a Texture2DArray to support instanced drawing
    glCreateTextures(GL_TEXTURE_2D, 1, &_canvas_background);
    glTextureParameteri(_canvas_background, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_canvas_background, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_canvas_background, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_canvas_background, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(_canvas_background, 1, GL_RGBA8, _canvas_width, _canvas_height);
    glTextureSubImage2D(_canvas_background, 0, 0, 0, _canvas_width, _canvas_height, GL_RGBA, GL_UNSIGNED_BYTE,
                        _canvas_background_pixels.data());
    glGenerateTextureMipmap(_canvas_background);

    const std::vector<std::uint32_t> _canvas_foreground_pixels(_canvas_width * _canvas_height, 0x00000000);

    glCreateTextures(GL_TEXTURE_2D, 1, &_canvas_foreground);
    glTextureParameteri(_canvas_foreground, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_canvas_foreground, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_canvas_foreground, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_canvas_foreground, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(_canvas_foreground, 1, GL_RGBA32F, _canvas_width, _canvas_height);
    glTextureSubImage2D(_canvas_foreground, 0, 0, 0, _canvas_width, _canvas_height, GL_RGBA, GL_UNSIGNED_BYTE,
                        _canvas_foreground_pixels.data());

    // Create forground texture

    // Compile shaders
    _canvas_program.Compile({"./data/shaders/canvas.frag", "./data/shaders/canvas.vert"});
    _brush_compute_program.Compile({"./data/shaders/brush.comp"});
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
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

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
        UpdateElapsedTime();
        glfwPollEvents();

        glfwGetCursorPos(_window, &_mouse_pos_current.x, &_mouse_pos_current.y);
        _mouse_delta = glm::dvec2(glm::vec2(_mouse_pos_previous - _mouse_pos_current));
        _mouse_pos_previous = _mouse_pos_current;

        Update();
        Render();
    }
}

void Mashiro::Update() {
    glfwSetWindowTitle(_window, std::format("{:.2f}ms", _elapsedTime * 1000.0).c_str());

    if (_using_tool) {
        switch (_cursor_state) {
        case State::Panning:
            _camera_pos += glm::vec2(_mouse_delta) / glm::vec2(-(float)_width, (float)_height);
            UpdateCamera();
            break;
        case State::Rotating:
            _camera_rot += _mouse_delta.y * 0.1;
            _camera_rot = std::fmodf(_camera_rot, 360);
            if (_camera_rot < 0) {
                _camera_rot += 360.0f;
            }
            UpdateCamera();
            break;
        case State::Zooming:
            _camera_zoom += _mouse_delta.y * 0.001;
            _camera_zoom = std::min(std::max(_camera_zoom, 0.2), 10.0);
            spdlog::info("zoom: {}", _camera_zoom);
            UpdateCamera();
            break;
        case State::Painting:
            _brush_compute_program.Bind();
            glUniform1i(_brush_compute_program._uniforms["focused"].location, 1);
            glBindImageTexture(0, _canvas_foreground, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(_canvas_width / 32, _canvas_height / 32, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            break;
        default:
            break;
        }
    } else {
        if (_cursor_state == State::Painting) {
            _brush_compute_program.Bind();
            glUniform1i(_brush_compute_program._uniforms["focused"].location, 0);
            glBindImageTexture(0, _canvas_foreground, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(_canvas_width / 32, _canvas_height / 32, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }
}

void Mashiro::Render() {
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _canvas_program.Bind();

    auto model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(512.0f / _width, 512.0f / _height, 1.0f));

    const auto camera_mv = _camera_projection * _camera_view * model;
    glUniformMatrix4fv(_canvas_program._uniforms["MV"].location, 1, GL_FALSE, glm::value_ptr(camera_mv));
    glUniform1i(_canvas_program._uniforms["canvas_background"].location, 0);
    glUniform1i(_canvas_program._uniforms["canvas_foreground"].location, 1);

    glBindTextureUnit(0, _canvas_background);
    glBindTextureUnit(1, _canvas_foreground);

    glBindVertexArray(_canvas_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window);
}

#pragma region GLFW Callbacks
static Mashiro *GetGlfwWindowUserPointer(GLFWwindow *window) {
    auto pointer = glfwGetWindowUserPointer(window);
    if (!pointer) {
        throw std::runtime_error("Missing window user pointer");
    }
    return static_cast<Mashiro *>(pointer);
}

void Mashiro::OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                    GLchar const *message, void const *user_param) {
    // spdlog::warn("Implement opengl callback");
    spdlog::info("[OPENGL]: {}", message);
}

void Mashiro::GLFWErrorCallback(int error, const char *description) {
    spdlog::error("[GLFW]: {} {}", error, description);
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
    mashiro->_width = width;
    mashiro->_height = height;

    glViewport(0, 0, width, height);

    float ortho_size = 1.0f;

    float aspect_ratio = (float)width / (float)height;
    if (aspect_ratio >= 1.0f) {
        float left = -ortho_size * aspect_ratio;
        float right = ortho_size * aspect_ratio;
        float bottom = -ortho_size;
        float top = ortho_size;
        mashiro->_camera_projection = glm::ortho(left, right, bottom, top);
    } else {
        float left = -ortho_size;
        float right = ortho_size;
        float bottom = -ortho_size / aspect_ratio;
        float top = ortho_size / aspect_ratio;
        mashiro->_camera_projection = glm::ortho(left, right, bottom, top);
    }

    mashiro->_camera_projection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size);
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
    mashiro->Render();
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

void Mashiro::UpdateCamera() {
    // auto view = glm::mat4(1.0f);
    // view = glm::translate(view, glm::vec3(_camera_pos, 0.0f));
    // view = glm::rotate(view, glm::radians((float)_camera_rot), glm::vec3(0.0, 0.0, 1.0));
    // view = glm::scale(view, glm::vec3((float)_camera_zoom));

    _camera_view = glm::mat4(1.0f);
    _camera_view = glm::translate(_camera_view, glm::vec3(_camera_pos, 0.0f));
    _camera_view = glm::rotate(_camera_view, glm::radians((float)_camera_rot), glm::vec3(0.0, 0.0, 1.0));
    _camera_view = glm::scale(_camera_view, glm::vec3((float)_camera_zoom));
}

void Mashiro::GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto mashiro = GetGlfwWindowUserPointer(window);

    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        mashiro->ToggleFullscreen();
    }

    if (action != GLFW_REPEAT) {
        spdlog::info("key:{}, scancode:{}, action:{}, mods:{} ", key, scancode, action, mods);
    }

    // FIXME: Fix bug when exiting and re-entering window (the cursor does not keep the correct state)

    // mashiro->_rotating = false;
    // mashiro->_panning = false;
    // mashiro->_zooming = false;

    if (action != GLFW_REPEAT) {
        if (key == GLFW_KEY_SPACE) {
            mashiro->_panning = action;

            if (mods & GLFW_MOD_SHIFT) {
                mashiro->_rotating = action;
            }

            if (mods & GLFW_MOD_CONTROL) {
                mashiro->_zooming = action;
            }
        }

        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
            mashiro->_rotating = action;
        }

        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
            mashiro->_zooming = action;
        }
    }

    if (mashiro->_zooming) {
        mashiro->SetCursorState(State::Zooming);
    } else if (mashiro->_rotating) {
        mashiro->SetCursorState(State::Rotating);
    } else if (mashiro->_panning) {
        mashiro->SetCursorState(State::Panning);
    } else {
        mashiro->SetCursorState(State::Painting);
    }

    /*

    if (key == GLFW_KEY_SPACE && mods & GLFW_MOD_SHIFT && action == GLFW_PRESS) {
        mashiro->SetCursorState(State::Rotating);
        spdlog::warn("State::Rotating");
    }

    if (key == GLFW_KEY_SPACE && mods & GLFW_MOD_CONTROL && action == GLFW_PRESS) {
        mashiro->SetCursorState(State::Zooming);
        spdlog::warn("State::Zooming");
    }

    if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
        mashiro->SetCursorState(State::Painting);
        spdlog::warn("State::Painting");
    }*/
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

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mashiro->_using_tool = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        if (action == GLFW_RELEASE) {
            mashiro->_using_tool = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mashiro->SetCursorState(mashiro->_cursor_state);
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }
    }
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
