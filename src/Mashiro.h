#pragma once

#include <memory>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Mashiro {
  public:
    Mashiro();
    ~Mashiro();
    void Run();


  private:
    void OnResize(int width, int height);
    void OnRender();

    void InitOpenGL();
    void InitConsole();
    void InitGLFW();
    void InitWindow();
    void UpdateElapsedTime();

  private:
    GLFWwindow *_window;

    std::string _title;
    int _width;
    int _height;

    double _elapsed_time;
    double _last_time;
    double _current_time;

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