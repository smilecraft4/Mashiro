#include "Window.h"
#include "App.h"
#include "Log.h"
#include "Resource.h"

#include <format>
#include <stdexcept>

// TODO: Maybe move the initialisation of OpenGL to the renderer rather than here, like Renderer::Init(HWND)

constexpr auto WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
constexpr auto WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
constexpr auto WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
constexpr auto WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext, const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList,
                                                 UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

Window::Window(int width, int height, const tstring &title)
    : _width(width), _height(height), _title(title), _dpi(96.0f), _fullscreen(false) {

    _hdc = nullptr;
    _hrc = nullptr;

    constexpr DWORD ex_style = 0;
    constexpr DWORD style = WS_OVERLAPPEDWINDOW;

    RECT rect{};
    rect.right = _width;
    rect.bottom = _height;
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);
    const int client_width = rect.right - rect.left;
    const int client_height = rect.bottom - rect.top;

    _hwnd = CreateWindowEx(ex_style, App::Get()->_window_class->Name().c_str(), _title.c_str(), style, CW_USEDEFAULT,
                           CW_USEDEFAULT, client_width, client_height, nullptr, nullptr,
                           App::Get()->_window_class->Instance(), this);
    if (!_hwnd) {
        WIN32_CHECK(GetLastError());
    }
}

void Window::Show() {
    ShowWindow(_hwnd, SW_NORMAL);

    _save_window_info._maximized = IsZoomed(_hwnd);
    _save_window_info._style = GetWindowLong(_hwnd, GWL_STYLE);
    _save_window_info._ex_style = GetWindowLong(_hwnd, GWL_EXSTYLE);
    GetWindowRect(_hwnd, &_save_window_info._rect);
    _x = _save_window_info._rect.left;
    _y = _save_window_info._rect.top;
}

Window::~Window() {
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(_hrc);
    ReleaseDC(_hwnd, _hdc);
    DestroyWindow(_hwnd);
}

LRESULT Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LOG_TRACE(std::format(TEXT("{}"), msg));

    _hwnd = hwnd;
    auto app = App::Get();

    LRESULT result = 0;
    if (app->_inputs->HandleEvents(hwnd, msg, wparam, lparam, &result)) {
        return result;
    }

    // Accelerators (Shortcuts)
    if (msg == WM_COMMAND) {
        switch (LOWORD(wparam)) {
        case ID_EXIT: {
            app->Exit();
            return 0;
        }
        case ID_SAVE: {
            app->Save();
            return 0;
        }
        case ID_SAVE_AS: {
            app->SaveAs();
            return 0;
        }
        case ID_OPEN: {
            app->Open();
            return 0;
        }
        case ID_NEW: {
            app->New();
            return 0;
        }
        case ID_TOGGLE_FULLSCREEN: {
            ToggleFullscren();
            return 0;
        }
        case ID_REFRESH: {
            return 0;
            app->Refresh();
        }
        case ID_MOVE: {
            return 0;
            app->SetNavigationMode();
        }
        case ID_BRUSH: {
            app->SetPaintingMode();
            return 0;
            app->_brush->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        case ID_ERASER: {
            app->SetPaintingMode();
            return 0;
            app->_brush->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        default:
            break;
        }
    }

    // Mashiro events
    switch (msg) {
    case MS_STYLUSMOVE: {
        LOG_INFO(TEXT("MS_STYLUSMOVE"));
        return 0;
    }
    case MS_STYLUSBUTTON: {
        LOG_INFO(TEXT("MS_STYLUSBUTTON"));
        return 0;
    }
    case MS_STYLUSDOWN: {
        LOG_INFO(TEXT("MS_STYLUSDOWN"));
        return 0;
    }
    case MS_STYLUSUP: {
        LOG_INFO(TEXT("MS_STYLUSUP"));
        return 0;
    }
    case MS_STYLUSENTER: {
        LOG_INFO(TEXT("MS_STYLUSENTER"));
        return 0;
    }
    case MS_STYLUSEXIT: {
        LOG_INFO(TEXT("MS_STYLUSEXIT"));
        return 0;
    }
    case MS_STYLUSHOVER: {
        LOG_INFO(TEXT("MS_STYLUSHOVER"));
        return 0;
    }
    case MS_STYLUSWHEEL: {
        LOG_INFO(TEXT("MS_STYLUSWHEEL"));
        return 0;
    }
    default:
        break;
    }

    // Window events
    switch (msg) {
    case WM_CREATE: {
        _hwnd = hwnd;
        // TODO use a more sensible place and name for this, it is quite hidden here
        Create();
        return 0;
    }
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT: {
        if (!app->_file || !app->_canvas) {
            break;
        }
        Render();
        return 0;
    }
    case WM_MOVE: {
        Move((short)LOWORD(lparam), (short)HIWORD(lparam));
        Render();
        return 0;
    }
    case WM_SIZE: {
        Resize(LOWORD(lparam), HIWORD(lparam));
        Render();
        return 0;
    }
    case WM_CLOSE: {
        app->Exit();
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return DefWindowProc(_hwnd, msg, wparam, lparam);
}

void Window::Create() {
    PIXELFORMATDESCRIPTOR pf_dscr{};
    pf_dscr.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pf_dscr.nVersion = 1;
    pf_dscr.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pf_dscr.iPixelType = PFD_TYPE_RGBA;
    pf_dscr.cColorBits = 32;

    _hdc = GetDC(_hwnd);
    const auto pf = ChoosePixelFormat(_hdc, &pf_dscr);
    if (!pf) {
        throw std::runtime_error("Failed to choose pixel format");
    }

    if (!SetPixelFormat(_hdc, pf, &pf_dscr)) {
        throw std::runtime_error("Failed to set pixel format");
    }

    DescribePixelFormat(_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pf_dscr);

    int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,    4, WGL_CONTEXT_MINOR_VERSION_ARB, 6, WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0,
    };

    _hrc = wglCreateContextAttribsARB(_hdc, 0, gl33_attribs);
    if (!_hrc) {
        throw std::runtime_error("Failed to create OpenGL 3.3 context.");
    }

    if (!wglMakeCurrent(_hdc, _hrc)) {
        throw std::runtime_error("Failed to activate OpenGL 3.3 rendering context.");
    }

    if (!gladLoadGL()) {
        throw std::runtime_error("Failed to load glad");
    }

    LOG_INFO(std::format(TEXT("OpenGL version: {}"), ConvertString((char *)glGetString(GL_VERSION))));

    typedef BOOL(APIENTRY * PFNWGLSWAPINTERVALPROC)(int);
    PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(0);
    }
    App::Get()->Init(_hwnd);
}

void Window::Update() noexcept {
}

void Window::Resize(int width, int height) {
    _width = width;
    _height = height;

    glViewport(0, 0, _width, _height);
    App::Get()->_viewport->SetSize(glm::ivec2(width, height));
    App::Get()->_framebuffer->Resize(width, height);
}

void Window::Move(int x, int y) {
    _x = x;
    _y = y;
}

void Window::Render() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render a simple tris with reloadale program
    App::Get()->Render();

    if (!SwapBuffers(_hdc)) {
        throw std::runtime_error("Failed to swap buffer");
    }
}

// FIXME: need to be heavely bugfixed and tweaked for better behaviour
// https://chromium.googlesource.com/chromium/src/+/refs/heads/main/ui/views/win/fullscreen_handler.cc
void Window::ToggleFullscren() noexcept {
    if (!_fullscreen) {
        _save_window_info._maximized = IsZoomed(_hwnd);
        if (_save_window_info._maximized) {
            SendMessage(_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
            _save_window_info._style = GetWindowLong(_hwnd, GWL_STYLE);
            _save_window_info._ex_style = GetWindowLong(_hwnd, GWL_EXSTYLE);
            GetWindowRect(_hwnd, &_save_window_info._rect);
            _x = _save_window_info._rect.left;
            _y = _save_window_info._rect.top;
        }
    }

    _fullscreen = !_fullscreen;

    if (_fullscreen) {
        SetWindowLong(_hwnd, GWL_STYLE, _save_window_info._style & ~(WS_CAPTION | WS_THICKFRAME));
        SetWindowLong(_hwnd, GWL_EXSTYLE,
                      _save_window_info._ex_style &
                          ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
        MONITORINFO monitor_info{};
        monitor_info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST), &monitor_info);
        const auto rect = monitor_info.rcMonitor;
        SetWindowPos(_hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    } else {
        SetWindowLong(_hwnd, GWL_STYLE, _save_window_info._style);
        SetWindowLong(_hwnd, GWL_EXSTYLE, _save_window_info._ex_style);
        const auto rect = _save_window_info._rect;
        SetWindowPos(_hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        if (_save_window_info._maximized) {
            SendMessage(_hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
    }
}

HWND Window::Hwnd() const noexcept {
    return _hwnd;
}

HDC Window::Hdc() const noexcept {
    return _hdc;
}

LRESULT CALLBACK WindowClass::WindowClassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    Window *window{};
    if (msg == WM_CREATE) {
        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lparam);
        window = static_cast<Window *>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
        window = reinterpret_cast<Window *>(ptr);
    }

    if (window) {
        return window->WindowProc(hwnd, msg, wparam, lparam);
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

WindowClass::WindowClass(HINSTANCE instance, const tstring &name) : _instance(instance) {

    _accel = LoadAccelerators(_instance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    const tstring menu_name = std::format(TEXT("{}Menu"), name).c_str();
    const tstring class_name = std::format(TEXT("{}Class"), name).c_str();

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc = WindowClassProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    ;
    wc.lpszMenuName = menu_name.c_str();
    wc.lpszClassName = class_name.c_str();
    wc.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(ID_ICON));
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(ID_ICON));

    _atom = RegisterClassEx(&wc);
    if (!_atom) {
        throw std::runtime_error("RegisterClass failed");
    }

    _name = class_name;

    HWND dummy_window = CreateWindowEx(0, class_name.c_str(), TEXT("Dummy OpenGL Window"), 0, CW_USEDEFAULT,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

    if (!dummy_window) {
        throw std::runtime_error("Failed to create  dummy window");
    }

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format) {
        throw std::runtime_error("Failed to find a suitable pixel format");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
        throw std::runtime_error("Failed to set the pixel format");
    }

    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) {
        throw std::runtime_error("Failed to create a dummy OpenGL rendering context");
    }

    if (!wglMakeCurrent(dummy_dc, dummy_context)) {
        throw std::runtime_error("Failed to activate dummy OpenGL rendering context.");
    }

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type *)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type *)wglGetProcAddress("wglChoosePixelFormatARB");

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
}

WindowClass::~WindowClass() {
    UnregisterClass(_name.c_str(), _instance);
}

ATOM WindowClass::Atom() const noexcept {
    return _atom;
}

tstring WindowClass::Name() const noexcept {
    return _name;
}

HINSTANCE WindowClass::Instance() const noexcept {
    return _instance;
}

HACCEL WindowClass::Accel() const noexcept {
    return _accel;
}
