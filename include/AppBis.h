#pragma once

class Tool {};
class Preferences {};
class Stylus {};

// Handle the window
// Handle the runnning of the application
class App {
  public:
    static App *Get();

    App(const App &) = delete;
    App(App &&) = delete;
    App &operator=(const App &) = delete;
    App &operator=(App &&) = delete;

    App(HINSTANCE hInstance, std::vector<std::wstring> args);
    ~App();
    void Run();

    void Move(int x, int y);
    void Resize(int width, int height);

    void Show();
    void Hide();
    void Maximize();
    void Minimize();
    void Fullscreen();

    void Update();
    void Render();
    void Refresh();

    void New();
    void Open();
    void Exit();
    void Save();
    void SaveAs();

  private:
    std::vector<std::wstring> _args;

    // Win32 Window
    HINSTANCE _instance;
    LPWSTR _class_name;
    DWORD _style;
    DWORD _ex_style;
    HWND _hwnd;
    HDC _hdc;
    HGLRC _hglrc;
    HACCEL _accel;

    bool _fullscreen;
    RECT _window_rect;
    std::wstring _title;

    Preferences _preferences;
};
