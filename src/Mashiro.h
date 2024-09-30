#pragma once

#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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


class Mashiro {
  public:
    Mashiro();
    void InitImGui();
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
    void OnResize(int width, int height);
    void Render();

    void InitOpenGL();
    void InitConsole();
    void InitGLFW();
    void InitWindow();
    void InitCursors();
    void SetCursorState(State new_state);
    void UpdateElapsedTime();
    void ToggleFullscreen();

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


  private:
#pragma region GLFW Callbacks
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