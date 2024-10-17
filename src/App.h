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
#include "Stylus.h"
#include "File.h"
#include "Settings.h"

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

    cmrc::embedded_filesystem _data;
    std::unique_ptr<Viewport> _viewport;
    std::unique_ptr<Brush> _brush;
    std::unique_ptr<Stylus> _stylus;
    std::unique_ptr<Canvas> _canvas;
    std::unique_ptr<File> _file;
    std::unique_ptr<Settings> _settings;

  protected:
    bool InitGLFW();
    bool InitOpenGL();

    void Render();
    void ToggleFullscreen();

    void Paint();
    void Navigate();

    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void ButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void FramebufferSize(GLFWwindow *window, int width, int height);

  private:
    int _width;
    int _height;
    std::string _title;

    GLFWwindow *_window;
    GLFWmonitor *_monitor;

    glm::dvec2 _cursor_previous;



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

    bool _fullscreen;

    struct Painting {
        bool enabled;
        glm::vec4 color;
        glm::vec2 cursor_pos;
        glm::vec2 previous_pos;
    } _painting;

    struct Navigation {
        bool enabled;
        float zoom_sens;
        float rotate_sens;
        float pan_sensitivity;

        glm::vec2 delta_mouse;
        glm::vec2 cursor_previous;
        glm::vec2 cursor_current;

        bool zooming;
        bool rotating;
        bool panning;
        bool using_hand;
    } _navigation;
};