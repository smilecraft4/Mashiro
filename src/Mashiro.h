#pragma once

#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Program.h"

/*
 * TODO
 * - Draw a square
 * - Create a texture on the square
 * - Paint in realtime on the square
 * - Have the Painting be persistant by opening a file with format .mshio
 * - Auto-save ?
 * - Move the viewport
 * - Zoom the viewport
 * - Rotate the viewport
 */

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

using Element = GLuint;

class Mashiro {
  public:
    Mashiro();
    ~Mashiro();
    void Run();


    enum State {
        Painting,
        Erasing,
        Selecting,
        Panning,
        Zooming,
        Rotating,
    };

  private:
    void Render();
    void Update();

    void InitImGui();
    void InitOpenGL();
    void InitConsole();
    void InitGLFW();
    void InitWindow();
    void InitCursors();

    void InitCanvas();

    void SetCursorState(State new_state);
    void UpdateElapsedTime();
    void ToggleFullscreen();

    void UpdateCamera();

  private:
    GLFWwindow *_window = NULL;
    GLFWmonitor *_monitor = NULL;
    GLFWcursor *_cursor_select = NULL;
    GLFWcursor *_cursor_paint = NULL;
    GLFWcursor *_cursor_erase = NULL;
    GLFWcursor *_cursor_pan = NULL;
    GLFWcursor *_cursor_zoom = NULL;
    GLFWcursor *_cursor_rotate = NULL;

    std::string _title = "Mashiro";
    int _width = 800;
    int _height = 600;

    double _elapsedTime = 0.0;
    double _lastTime = 0.0;
    double _currentTime = 0.0;

    int _windowed_x = 0;
    int _windowed_y = 0;
    int _windowed_width = 800;
    int _windowed_height = 600;

    // TODO: Have a more proper state manager
    State _cursor_state = State::Painting;

    bool _imgui_initialized = false;
    bool _fullscreen = false;
    bool _using_tool = false;

    Program _canvas_program{};
    Program _brush_compute_program{};

    // TODO: replace by mesh class
    GLuint _canvas_vao;
    GLuint _canvas_vbo;
    GLuint _canvas_ebo;

    // TODO: replace by texture class
    const int _canvas_width = 1024;
    const int _canvas_height = 1024;
    GLuint _canvas_background = 0;
    GLuint _canvas_foreground = 0;

    // TODO: replace by model class
    glm::mat4 _canvas_model = glm::mat4(1.0f);

    // TODO: replace by Camera class
    glm::vec2 _camera_pos = glm::vec2(0.0f);
    double _camera_zoom = 1.0;
    double _camera_rot = 0.0;
    glm::mat4 _camera_projection = glm::mat4(1.0f);
    glm::mat4 _camera_view = glm::mat4(1.0f);

  private:
#pragma region GLFW Callbacks
    static void OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                      GLchar const *message, void const *user_param);
    static void GLFWErrorCallback(int error, const char *description);
    static void GLFWMonitorCallback(GLFWmonitor *monitor, int event);
    static void GLFWWindowCloseCallback(GLFWwindow *window);
    static void GLFWWindowSizeCallback(GLFWwindow *window, int width, int height);
    static void GLFWFramebufferSizeCallback(GLFWwindow *window, int width, int height);
    static void GLFWWindowContentScaleCallback(GLFWwindow *window, float xscale, float yscale);
    static void GLFWWindowPosCallback(GLFWwindow *window, int xpos, int ypos);
    static void GLFWWindowIconifyCallback(GLFWwindow *window, int iconified);
    static void GLFWWindowMaximizeCallback(GLFWwindow *window, int maximized);
    static void GLFWWindowFocusCallback(GLFWwindow *window, int focused);
    static void GLFWWindowRefreshCallback(GLFWwindow *window);
    static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void GLFWCharCallback(GLFWwindow *window, unsigned int codepoint);
    static void GLFWCursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void GLFWCursorEnterCallback(GLFWwindow *window, int entered);
    static void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void GLFWScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void GLFWJoystickCallback(int jid, int event);
    static void GLFWDropCallback(GLFWwindow *window, int count, const char **paths);
#pragma endregion
};