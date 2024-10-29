#include "App.h"
#include "Log.h"
#include "Resource.h"

#include <ShObjIdl.h>

App *g_app = nullptr;

App::App(HINSTANCE instance, int show_cmd) : _instance(instance), _show_cmd(show_cmd) {
    // FIXME: Having only one global is alright and this one is really useful but the codebase may become to coupled
    // overal
    g_app = this; // Must be first

    LOG_INFO(TEXT("Mashiro starting"));

    _preferences = std::make_unique<Preferences>();

    SetPaintingMode();

    _stylus = std::make_unique<Stylus>();

    // InitSettings
    _window_class = std::make_unique<WindowClass>(_instance, TEXT("Mashiro"));
    _window = std::make_unique<Window>(800, 600, TEXT("Mashiro"));
}

App::~App() noexcept {
    //_canvas->Save(_file.get());
    //_file->Save();

    LOG_INFO(TEXT("Mashiro closing"));
}

void App::Run() const {
    LOG_INFO(TEXT("Mashiro running"));
    _window->Show();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        if (!TranslateAccelerator(_window->Hwnd(), _window_class->Accel(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (_file && _canvas) {
            _canvas->LazySave({0.0f, 0.0f}, _file.get());
        }
    }
}

void App::EnableBrush(bool enable) {
    _brush_enabled = enable;
}

void App::Init(HWND hwnd) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    _viewport = std::make_unique<Viewport>(glm::ivec2(800, 600));

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

    Canvas::Init();
    Brush::Init();
    Framebuffer::Init();

    _brush = std::make_unique<Brush>();
    _brush->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _brush->SetPressure(1.0f);
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

    //_brush->Render();
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

    hr = pfd->Show(App::Get()->_window->Hwnd());
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

    hr = pfd->Show(App::Get()->_window->Hwnd());
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

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());
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

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());
    return true;
}

bool App::Open() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window->Hwnd(), _instance, TEXT("Mashiro"),
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

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());

    _window->Render();
    return true;
}

bool App::New() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window->Hwnd(), _instance, TEXT("Mashiro"),
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

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());

    _window->Render();

    return true;
}

void App::Exit() {
    if (_file) {
        if (!_file->IsSaved() || !_canvas->IsSaved()) {
            int result;
            TaskDialog(_window->Hwnd(), _instance, TEXT("Mashiro"),
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

    DestroyWindow(_window->Hwnd());
}