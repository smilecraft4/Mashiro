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
}

App::~App() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void App::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwWaitEvents();
        Render();
    }
}

// TODO: enable Alpha blending
void App::Render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _canvas->Render();

    glfwSwapBuffers(_window);
}

void App::FramebufferSize(GLFWwindow *window, int width, int height) {
    App *app = (App *)glfwGetWindowUserPointer(window);
    assert(app && "Failed to retrieve window");

    app->_width = width;
    app->_height = height;
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