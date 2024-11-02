#include "pch.h"

#include "App.h"
#include "Log.h"
#include "Resource.h"

App *g_app = nullptr;

static LRESULT CALLBACK WindowClassProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    App *app{};
    if (msg == WM_CREATE) {
        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lparam);
        app = static_cast<App *>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
        app = reinterpret_cast<App *>(ptr);
    }

    if (app) {
        return app->AppWindowProc(hwnd, msg, wparam, lparam);
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

App::App(HINSTANCE instance, LPTSTR cmd_line, int show_cmd)
    : _instance(instance), _cmd_line(cmd_line), _cmd_show(show_cmd) {
    g_app = this; // Must be first

    _prefs = std::make_unique<Preferences>();
    _stylus = std::make_unique<Stylus>();

    // Create window class
    _window.accel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR1));

    _window.hwnd = InitWindow();
    _window.dc = GetDC(_window.hwnd);
    _window.glrc = InitRenderer(_window.dc);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    _viewport = std::make_unique<Viewport>(glm::ivec2(800, 600));

    Canvas::Init();
    Brush::Init();
    Framebuffer::Init();

    _mesh = Mesh::Create(TEXT("Quad"));
    _program = Program::Create(TEXT("test"));
    _program->AddShader("data/default.frag", GL_FRAGMENT_SHADER);
    _program->AddShader("data/default.vert", GL_VERTEX_SHADER);
    _program->Compile();

    _texture = Texture::Create(TEXT("Test"), 256, 256);
    std::vector<std::uint32_t> pixels(256 * 256, 0xFFFF00FF);
    _texture->SetPixels(pixels);
    _texture->GenerateMipmaps();

    _framebuffer = Framebuffer::Create(TEXT("Canvas framebuffer"), 800, 600);

    _brush = std::make_unique<Brush>();
    _brush->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _brush->SetPressure(1.0f);

    New();
}

HWND App::InitWindow() {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc = WindowClassProc;
    wc.hInstance = _instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"MashiroClass";
    wc.hIconSm = LoadIcon(_instance, MAKEINTRESOURCE(ID_ICON));
    wc.hIcon = LoadIcon(_instance, MAKEINTRESOURCE(ID_ICON));

    ATOM class_result = RegisterClassEx(&wc);
    if (!class_result) {
        LOG_CRITICAL(L"Failed to register window class");
        exit(-1);
    }

    // Create window

    constexpr DWORD ex_style = 0;
    constexpr DWORD style = WS_OVERLAPPEDWINDOW;

    // Load window size from preferences/last sessions
    RECT rect{};
    rect.right = 800;
    rect.bottom = 600;
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    HWND hwnd = CreateWindowEx(ex_style, wc.lpszClassName, L"Mashiro", style, CW_USEDEFAULT, CW_USEDEFAULT,
                               rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, wc.hInstance, this);
    if (!hwnd) {
        LOG_CRITICAL(L"Failed to create window");
        exit(-1);
    }

    return hwnd;
}

// TODO
HGLRC App::InitOpenGL() {
    return HGLRC();
}

void App::Resize(int width, int height) {
    _window.width = width;
    _window.height = height;

    glViewport(0, 0, _window.width, _window.height);
    App::Get()->_viewport->SetSize(glm::ivec2(_window.width, _window.height));
    App::Get()->_framebuffer->Resize(_window.width, _window.height);
}

void App::Move(int x, int y) {
    _window.x = x;
    _window.y = y;
}

App::~App() {
    UnregisterClass(L"MashiroClass", _instance);
    LOG_INFO(TEXT("Mashiro closing"));
}

int App::Run() {
    LOG_INFO(TEXT("Mashiro running"));

    ShowWindow(_window.hwnd, _cmd_show);
    UpdateWindow(_window.hwnd);

    MSG msg{};
    BOOL ret{};

    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) {
            return ret;
        } else {
            if (!TranslateAccelerator(_window.hwnd, _window.accel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return 0;
}

LRESULT App::AppWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LOG_TRACE(std::format(TEXT("{}"), msg));

    LRESULT result = 0;
    if (_stylus->HandleEvents(hwnd, msg, wparam, lparam, &result)) {
        return result;
    }

    // Accelerators (Shortcuts)
    if (msg == WM_COMMAND) {
        switch (LOWORD(wparam)) {
        case ID_EXIT: {
            Exit();
            return 0;
        }
        case ID_SAVE: {
            Save();
            return 0;
        }
        case ID_SAVE_AS: {
            SaveAs();
            return 0;
        }
        case ID_OPEN: {
            Open();
            return 0;
        }
        case ID_NEW: {
            New();
            return 0;
        }
        case ID_REFRESH: {
            return 0;
            Refresh();
        }
        case ID_MOVE: {
            return 0;
            SetNavigationMode();
        }
        case ID_BRUSH: {
            SetPaintingMode();
            return 0;
            _brush->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        case ID_ERASER: {
            SetPaintingMode();
            return 0;
            _brush->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
        default:
            break;
        }
    }

    // Mashiro events
    switch (msg) {
    case MS_STYLUSMOVE: {
        // LOG_INFO(TEXT("MS_STYLUSMOVE"));
        if (_stylus->_left) {
            Brush::BrushData start = _brush->GetBrushData();
            Brush::BrushData end = _brush->GetBrushData();

            start.position.x = _stylus->_previous_packet.x;
            start.position.y = _stylus->_previous_packet.y;
            end.position.x = _stylus->_current_packet.x;
            end.position.y = _stylus->_current_packet.y;

            start.position = glm::vec4((glm::vec2(start.position) / glm::vec2(_viewport->GetSize()) - glm::vec2(0.5f)) *
                                           glm::vec2(2.0f, -2.0f),
                                       0.0, 1.0);

            end.position = glm::vec4((glm::vec2(end.position) / glm::vec2(_viewport->GetSize()) - glm::vec2(0.5f)) *
                                         glm::vec2(2.0f, -2.0f),
                                     0.0, 1.0);

            end.position = glm::inverse(_viewport->_matrices.view) * glm::inverse(_viewport->_matrices.proj) *
                           glm::vec4(end.position, 0.0, 1.0);
            start.position = glm::inverse(_viewport->_matrices.view) * glm::inverse(_viewport->_matrices.proj) *
                             glm::vec4(start.position, 0.0, 1.0);

            _brush->PaintLine(_canvas.get(), start, end, 0.25f);
        }
        return 0;
    }
    case MS_STYLUSBUTTON: {
        // LOG_INFO(TEXT("MS_STYLUSBUTTON"));
        return 0;
    }
    case MS_STYLUSDOWN: {
        // LOG_INFO(TEXT("MS_STYLUSDOWN"));
        return 0;
    }
    case MS_STYLUSUP: {
        // LOG_INFO(TEXT("MS_STYLUSUP"));
        _brush_enabled = false;
        return 0;
    }
    case MS_STYLUSENTER: {
        // LOG_INFO(TEXT("MS_STYLUSENTER"));
        return 0;
    }
    case MS_STYLUSEXIT: {
        // LOG_INFO(TEXT("MS_STYLUSEXIT"));
        return 0;
    }
    case MS_STYLUSHOVER: {
        // LOG_INFO(TEXT("MS_STYLUSHOVER"));
        return 0;
    }
    case MS_STYLUSWHEEL: {
        // LOG_INFO(TEXT("MS_STYLUSWHEEL"));
        return 0;
    }
    default:
        break;
    }

    // Window events
    switch (msg) {
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT: {
        if (!_file || !_canvas) {
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
        Exit();
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return DefWindowProc(_window.hwnd, msg, wparam, lparam);
}

void App::EnableBrush(bool enable) {
    _brush_enabled = enable;
}

void App::Update() {
    _canvas->LazySave({0.0f, 0.0f}, _file.get());
}

void App::Render() {
    _framebuffer->Bind();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (_canvas) {
        _canvas->Render(_viewport.get());
    }

    _framebuffer->Unbind();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    _framebuffer->Render();

    SwapBuffers(_window.dc);
}

void App::Refresh() {
    _canvas.reset();
    _canvas = Canvas::New();
    _brush->Refresh();
    _program->Compile();
}

void App::SetNavigationMode() {
    _painting_mode = false;
    _navigation_mode = true;
}

void App::SetPaintingMode() {
    _painting_mode = true;
    _navigation_mode = false;
}

App *App::Get() {
    return g_app;
}

// TODO: Consider moving this to another file such as FileDialogs.h

static COMDLG_FILTERSPEC file_spec[] = {{TEXT("Mashiro files"), TEXT("*.msh")}, {TEXT("All files"), TEXT("*.*")}};

std::optional<std::filesystem::path> SaveAsDialog(std::filesystem::path filename) {

    IFileDialog *pfd{};
    IShellItem *psiResult{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));
    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(file_spec), file_spec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetFileName(filename.wstring().c_str());
    hr = pfd->SetDefaultExtension(L"msh");

    hr = pfd->Show(App::Get()->_window.hwnd);
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        return {};
    }

    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    psiResult->Release();
    pfd->Release();

    return path;
}

std::optional<std::filesystem::path> OpenDialog() {

    IFileDialog *pfd{};
    IShellItem *psiResult{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(file_spec), file_spec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetDefaultExtension(L"msh");

    hr = pfd->Show(App::Get()->_window.hwnd);
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        return {};
    }

    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    psiResult->Release();
    pfd->Release();

    return path;
}

// TODO: Better handle the communication between the file and the canvas

bool App::Save() {
    if (!_file) {
        return false;
    }

    if (_file->IsNew()) {
        return SaveAs();
    }

    if (_file->IsSaved() && _canvas->IsSaved()) {
        return true;
    }

    _canvas->Save(_file.get());
    _file->Save(_file->GetFilename());

    SetWindowText(_window.hwnd, _file->GetDisplayName().c_str());
    return true;
}

bool App::SaveAs() {
    if (!_file) {
        return false;
    }

    auto directory = _file->GetFilename().root_directory();
    if (!std::filesystem::exists(directory)) {
        directory = "./";
    }

    const auto filename = _file->GetFilename().filename();
    const auto path = SaveAsDialog(filename);
    if (!path.has_value()) {
        return false;
    }

    _file->Rename(path.value().filename());
    _canvas->Save(_file.get());
    _file->Save(path.value());

    SetWindowText(_window.hwnd, _file->GetDisplayName().c_str());
    return true;
}

bool App::Open() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window.hwnd, _instance, TEXT("Mashiro"),
                       std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(), TEXT("dsqdqs"),
                       TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON, TD_WARNING_ICON, &result);
            switch (result) {
            case IDYES:
                Save();
                break;
            case IDNO:
                break;
            case IDCANCEL:
                return false;
            }
        }
    }

    const auto path = OpenDialog();
    if (!path.has_value()) {
        return false;
    }

    _canvas.release();
    _file.release();

    _file = File::Open(path.value());
    _canvas = Canvas::Open(_file.get());

    SetWindowText(_window.hwnd, _file->GetDisplayName().c_str());

    Render();
    return true;
}

bool App::New() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window.hwnd, _instance, TEXT("Mashiro"),
                       std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(), TEXT("dsqdqs"),
                       TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON, TD_WARNING_ICON, &result);
            switch (result) {
            case IDYES: {
                const auto result = Save();
                if (!result) {
                    return false;
                }
                break;
            }
            case IDNO:
                break;
            case IDCANCEL:
                return false;
            }
        }
    }

    _canvas.release();
    _file.release();

    _file = File::New("unnamed.msh");
    _canvas = Canvas::Open(_file.get());

    SetWindowText(_window.hwnd, _file->GetDisplayName().c_str());

    Render();

    return true;
}

void App::Exit() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window.hwnd, _instance, TEXT("Mashiro"),
                       std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(), TEXT("dsqdqs"),
                       TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON, TD_WARNING_ICON, &result);
            switch (result) {
            case IDYES: {
                const auto result = Save();
                if (!result) {
                    return;
                }
                break;
            }
            case IDNO:
                break;
            case IDCANCEL:
                return;
            }
        }
    }

    DestroyWindow(_window.hwnd);
}