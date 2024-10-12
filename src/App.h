#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <string>
#include <glm/mat4x4.hpp>
#include <cmrc/cmrc.hpp>

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
    cmrc::embedded_filesystem _data;

    // TODO: move to a map or something like this if needed
    GLuint _ubo_matrices;

  private:
    void Render();
    static void FramebufferSize(GLFWwindow *window, int width, int height);

  private:
    int _width;
    int _height;
    std::string _title;

    glm::mat4 _projection;


    GLFWwindow *_window;

    std::unique_ptr<Canvas> _canvas;
    std::unique_ptr<Viewport> _viewport;
    // std::unique_ptr<Preferences> _preferences;
    // std::unique_ptr<Renderer> _renderer;
    // std::unique_ptr<Canvas> _canvas
    // std::unique_ptr<Tool> _tool;

};