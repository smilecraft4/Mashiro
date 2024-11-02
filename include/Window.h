#pragma once
#include "Renderer.h"

class WindowClass final {
  public:
    WindowClass(const WindowClass &) = delete;
    WindowClass(WindowClass &&) = delete;
    WindowClass &operator=(const WindowClass &) = delete;
    WindowClass &operator=(WindowClass &&) = delete;

    WindowClass(HINSTANCE instance, const std::wstring &name);
    ~WindowClass();

    static LRESULT CALLBACK WindowClassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    ATOM Atom() const noexcept;
    std::wstring Name() const noexcept;
    HINSTANCE Instance() const noexcept;
    HACCEL Accel() const noexcept;

  private:
    ATOM _atom;
    std::wstring _name;
    HINSTANCE _instance;
    HACCEL _accel;
};

/**
 * @brief The window is only responsible for displaying and receiving events
 *
 */
class Window final {
  public:
    friend class WindowClass;

    static void InitOpenGL();

    Window(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(const Window &) = delete;
    Window &operator=(Window &&) = delete;

    Window(int width, int height, const std::wstring &title);
    ~Window();

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void Show();
    void Render(Canvas *canvas);
    void ToggleFullscren() noexcept;

    HWND Hwnd() const noexcept;
    HDC Hdc() const noexcept;

  protected:
    void Create();
    void Update() noexcept;
    void Resize(int width, int height);
    void Move(int x, int y);

  private:
    struct SaveWindowInfo {
        bool _maximized;
        DWORD _style;
        DWORD _ex_style;
        RECT _rect;
    } _save_window_info;

    bool _fullscreen;
    int _x;
    int _y;
    int _width;
    int _height;
    float _dpi;
    std::wstring _title;

    HWND _hwnd;
    HDC _hdc;
    HGLRC _hGLrc;

    Viewport _viewport;
};
