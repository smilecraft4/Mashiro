#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cmrc/cmrc.hpp>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <string>

#include "Brush.h"
#include "Canvas.h"
#include "Viewport.h"

class App {
  public:
    App(const App &) = delete;
    App(App &&) = delete;
    App &operator=(const App &) = delete;
    App &operator=(App &&) = delete;

    App();
    ~App();

    void Run();
    void Update();

    // TODO: move to a map or something like this if needed
    GLuint _ubo_matrices;
    cmrc::embedded_filesystem _data;
    std::unique_ptr<Canvas> _canvas;

  private:
    void Render();
    void ToggleFullscreen();

    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void ButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void FramebufferSize(GLFWwindow *window, int width, int height);

  private:
    int _width;
    int _height;
    std::string _title;

    glm::mat4 _projection;

    GLFWwindow *_window;
    GLFWmonitor *_monitor;

    glm::dvec2 _cursor_previous;

    std::unique_ptr<Viewport> _viewport;
    std::unique_ptr<Brush> _brush;
    // std::unique_ptr<Preferences> _preferences;
    // std::unique_ptr<Renderer> _renderer;
    // std::unique_ptr<Canvas> _canvas
    // std::unique_ptr<Tool> _tool;

    struct WindowSize {
        int xpos;
        int ypos;
        int width;
        int height;
        bool maximized;
    } _saved_window_size;

    bool _painting;
    bool _fullscreen;

    bool _zooming;
    bool _rotating;
    bool _panning;
    bool _using_hand;
};