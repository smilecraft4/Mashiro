#include "Mashiro.h"

#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

static std::string LoadFile(std::filesystem::path filename) {
    return "";
}

Mashiro::Mashiro() {
    InitConsole();
    InitGLFW();
    InitWindow();
    InitOpenGL();
    // InitImGui();

    InitCanvas();

    UpdateCamera();
}

void Mashiro::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
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

    const std::vector<std::uint32_t> _canvas_background_pixels(_canvas_width * _canvas_height, 0xF00FFFFF);

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

        glfwSetWindowTitle(_window, std::format("{:.2f}ms", _elapsedTime * 1000.0).c_str());

        if (_using_tool) {
            if (_cursor_state == State::Painting) {
                _brush_compute_program.Bind();

                glm::vec4 brush_color = glm::vec4(sin(glfwGetTime()) / 2 + 0.5, 0.0f, 1.0f, 1.0f);

                glUniform4fv(_brush_compute_program._uniforms["brush_color"].location, 1, glm::value_ptr(brush_color));
                glBindImageTexture(0, _canvas_foreground, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

                glDispatchCompute(_canvas_width / 32, _canvas_height / 32, 1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            }
        }

        Render();
    }
}

void Mashiro::Render() {
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();
    //  ImGui::ShowDemoWindow();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _canvas_program.Bind();
    glUniformMatrix4fv(_canvas_program._uniforms["MV"].location, 1, GL_FALSE, glm::value_ptr(_camera_view));
    glUniform1i(_canvas_program._uniforms["canvas_background"].location, 0);
    glUniform1i(_canvas_program._uniforms["canvas_foreground"].location, 1);

    glBindTextureUnit(0, _canvas_background);
    glBindTextureUnit(1, _canvas_foreground);

    glBindVertexArray(_canvas_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

void Mashiro::OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                    GLchar const *message, void const *user_param) {
    // spdlog::warn("Implement opengl callback");
    spdlog::info("{}", message);
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
    auto view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(_camera_pos, 0.0f));
    view = glm::rotate(view, glm::radians((float)_camera_rot), glm::vec3(0.0, 0.0, 1.0));
    view = glm::scale(view, glm::vec3((float)_camera_zoom));
    _camera_view = view;
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

    if (mashiro->_cursor_state == State::Panning) {
        spdlog::info("{} {}", xpos, ypos);
        mashiro->_camera_pos += glm::vec2((float)xpos, (float)ypos) * 0.000001f;

        mashiro->UpdateCamera();
    }
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
        }
        if (action == GLFW_RELEASE) {
            mashiro->_using_tool = false;
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
