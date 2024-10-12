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

    assert(glfwInit() && "Failed to initialize GLFW");

    glfwSetErrorCallback(GlfwErrorCallback);

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
    if (_painting) {
        double cursor_x, cursor_y;
        int pos_x, pos_y, width, height;
        glfwGetCursorPos(_window, &cursor_x, &cursor_y);
        glfwGetWindowPos(_window, &pos_x, &pos_y);
        glfwGetWindowSize(_window, &width, &height);

        // convert cursor pos to screen_space [(pos_x, pos_y); (pos_x + width, pos_y + height)] --> [(-1,-1); (1,1)]
        glm::vec4 brush_pos = {(cursor_x) / (float)width * 2.0f - 1.0f,
                               ((cursor_y) / (float)height * 2.0f - 1.0f) * -1.0f, 0.0f, 0.0f};

        glm::vec2 test = glm::inverse(_viewport->_viewport) * glm::inverse(_projection) * brush_pos;

        _brush->SetPosition(test);
        _brush->Use(_canvas.get());
        glfwPostEmptyEvent();
    }
}

void App::Render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _canvas->Render();

    glfwSwapBuffers(_window);
}

void App::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    int state = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) | glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    if (state == GLFW_PRESS) {
        const auto hardness = app->_brush->GetHardness() + (float)yoffset * 0.01f;
        app->_brush->SetHardness(std::max(0.0f, std::min(1.0f, hardness)));
    } else {
        const auto radius = app->_brush->GetRadius() + (float)yoffset / 0.5f;
        app->_brush->SetRadius(std::max(0.0f, std::min(50.0f, radius)));
    }
}

void App::ButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !app->_painting) {
            app->_brush->SetColor({0.0f, 0.0f, 0.0f, 255.0f});
            app->_painting = true;
        } else if (action == GLFW_RELEASE) {
            app->_painting = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT ) {
        if (action == GLFW_PRESS && !app->_painting) {
            app->_brush->SetColor({255.0f, 255.0f, 255.0f, 127.0f});
            app->_painting = true;
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
    app->_projection =
        glm::orthoLH(-app->_width / 2.0f, app->_width / 2.0f, -app->_height / 2.0f, app->_height / 2.0f, 0.0f, 1.0f);

    glViewport(0, 0, app->_width, app->_height);
    glBindBuffer(GL_UNIFORM_BUFFER, app->_ubo_matrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(app->_projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    app->Render();
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