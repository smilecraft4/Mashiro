#pragma once

#include "Brush.h"
#include "Canvas.h"
#include "File.h"
#include "Preferences.h"
#include "Renderer.h"
#include "Stylus.h"
#include "Viewport.h"

// #include "Window.h"

// TODO: Change this from a app like this to a window
// TODO: Create wrapper for Wintab
// FIXME: Allow the mouse to be used

class App final {
  public:
    static App *Get();

    App(const App &) = delete;
    App(App &&) = delete;
    App &operator=(const App &) = delete;
    App &operator=(App &&) = delete;

    App(HINSTANCE instance, LPTSTR cmd_line, int show_cmd);
    ~App();
    int Run();

    LRESULT CALLBACK AppWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    bool Save();
    bool SaveAs();
    bool Open();
    bool New();
    void Exit();

    void EnableBrush(bool enable);

    void Update();
    void Render();
    void Refresh();

    void SetNavigationMode();
    void SetPaintingMode();

  private:
    HWND InitWindow();
    HGLRC InitOpenGL();
    void Resize(int width, int height);
    void Move(int x, int y);

  public:
    HINSTANCE _instance;
    std::wstring _cmd_line;
    int _cmd_show;
    bool _brush_enabled;

    struct Window {
        HACCEL accel;
        HWND hwnd;
        HDC dc;
        HGLRC glrc;

        int x;
        int y;
        int width;
        int height;
    } _window;

    std::unique_ptr<Preferences> _prefs;

    std::unique_ptr<File> _file;
    std::unique_ptr<Canvas> _canvas;
    std::unique_ptr<Viewport> _viewport;

    // TODO: Convert to tools
    std::unique_ptr<Brush> _brush;
    std::unique_ptr<Framebuffer> _framebuffer;

    // TODO: Delete all this unnecessary stuff
    std::unique_ptr<Uniformbuffer> _app_uniformbuffer;
    std::unique_ptr<Mesh> _mesh;
    std::unique_ptr<Texture> _texture;
    std::unique_ptr<Program> _program;

    // Create a hiearical finite state machine instead that handles keybinds shortcuts and everything here
    bool _painting_mode;
    bool _navigation_mode;

    std::unique_ptr<Stylus> _stylus;
};
