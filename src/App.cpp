#include "App.h"
#include "Log.h"
#include "Resource.h"

#include <ShObjIdl.h>

int App::nOpenContexts = 0;
int App::nAttachedDevices = 0;

HCTX App::_hctx = nullptr;
HCTX App::_hctxLast = nullptr;
double App::_scaleWidth = 0.0;
double App::_scaleHeight = 0.0;
RECT App::_clientRect = {0};
RECT App::_windowRect = {0};
MONITORINFO App::_monInfo = {0};

App *g_app = nullptr;

/// Asks Wintab for a data packet.  Normally would use this in response to
/// a non-Wintab event, such as a mouse event.  If new data received, the
/// drawing area is invalidated so that the data can be drawn.
///
void App::PollForPenData(HCTX hCtx_I, HWND hWnd_I, POINT &ptOld_I, UINT &prsOld_I, POINT &ptNew_O, UINT &prsNew_O) {
    PACKET pkts[MAX_PACKETS] = {0};

    // Get up to MAX_PACKETS from Wintab data packet cache per request.
    int numPackets = gpWTPacketsGet(hCtx_I, MAX_PACKETS, (LPVOID)pkts);

    for (int idx = 0; idx < numPackets; idx++) {
        PACKET *pkt = &pkts[idx];

        // WacomTrace("pkt: x,y,p: %i,%i,%i\n", pkt->pkX, pkt->pkY, pkt->pkNormalPressure);

        ptOld_I = ptNew_O;
        prsOld_I = prsNew_O;

        ptNew_O.x = pkt->pkX;
        ptNew_O.y = pkt->pkY;

        prsNew_O = pkt->pkNormalPressure;

        if (ptNew_O.x != ptOld_I.x || ptNew_O.y != ptOld_I.y || prsNew_O != prsOld_I) {
            InvalidateRect(hWnd_I, nullptr, false);
        }
    }
}

void DumpWintabContext(const LOGCONTEXTA &ctx_I) {
    WacomTrace("***********************************************\n");
    WacomTrace("Context:\n");
    WacomTrace("  lcName:      %s\n", ctx_I.lcName);
    WacomTrace("  lcOptions:   %i\n", ctx_I.lcOptions);
    WacomTrace("  lcStatus:    %i\n", ctx_I.lcStatus);
    WacomTrace("  lcLocks:     %i\n", ctx_I.lcLocks);
    WacomTrace("  lcMsgBase:   %i\n", ctx_I.lcMsgBase);
    WacomTrace("  lcDevice:    %i\n", ctx_I.lcDevice);
    WacomTrace("  lcPktRate:   %i\n", ctx_I.lcPktRate);
    WacomTrace("  lcPktData:   %i\n", ctx_I.lcPktData);
    WacomTrace("  lcPktMode:   %i\n", ctx_I.lcPktMode);
    WacomTrace("  lcMoveMask:  0x%X\n", ctx_I.lcMoveMask);
    WacomTrace("  lcBtnDnMask: 0x%X\n", ctx_I.lcBtnDnMask);
    WacomTrace("  lcBtnUpMask: 0x%X\n", ctx_I.lcBtnUpMask);
    WacomTrace("  lcInOrgX:    %i\n", ctx_I.lcInOrgX);
    WacomTrace("  lcInOrgY:    %i\n", ctx_I.lcInOrgY);
    WacomTrace("  lcInOrgZ:    %i\n", ctx_I.lcInOrgZ);
    WacomTrace("  lcInExtX:    %i\n", ctx_I.lcInExtX);
    WacomTrace("  lcInExtY:    %i\n", ctx_I.lcInExtY);
    WacomTrace("  lcInExtZ:    %i\n", ctx_I.lcInExtZ);
    WacomTrace("  lcOutOrgX:   %i\n", ctx_I.lcOutOrgX);
    WacomTrace("  lcOutOrgY:   %i\n", ctx_I.lcOutOrgY);
    WacomTrace("  lcOutOrgZ:   %i\n", ctx_I.lcOutOrgZ);
    WacomTrace("  lcOutExtX:   %i\n", ctx_I.lcOutExtX);
    WacomTrace("  lcOutExtY:   %i\n", ctx_I.lcOutExtY);
    WacomTrace("  lcOutExtZ:   %i\n", ctx_I.lcOutExtZ);
    WacomTrace("  lcSensX:     %i\n", ctx_I.lcSensX);
    WacomTrace("  lcSensY:     %i\n", ctx_I.lcSensY);
    WacomTrace("  lcSensZ:     %i\n", ctx_I.lcSensZ);
    WacomTrace("  lcSysMode:   %i\n", ctx_I.lcSysMode);
    WacomTrace("  lcSysOrgX:   %i\n", ctx_I.lcSysOrgX);
    WacomTrace("  lcSysOrgY:   %i\n", ctx_I.lcSysOrgY);
    WacomTrace("  lcSysExtX:   %i\n", ctx_I.lcSysExtX);
    WacomTrace("  lcSysExtY:   %i\n", ctx_I.lcSysExtY);
    WacomTrace("  lcSysSensX:  %i\n", ctx_I.lcSysSensX);
    WacomTrace("  lcSysSensY:  %i\n", ctx_I.lcSysSensY);
    WacomTrace("***********************************************\n");
}

App::App(HINSTANCE instance, int show_cmd) : _instance(instance), _show_cmd(show_cmd) {
    g_app = this; // Must be first

    Log::Info(TEXT("Mashiro starting"));

    _preferences = std::make_unique<Preferences>();

    if (!LoadWintab()) {
        throw std::runtime_error("Failed to initialize wintab.dll");
    }

    /* check if WinTab available. */
    if (!gpWTInfoA(0, 0, nullptr)) {
        ShowError("WinTab Services Not Available.");
    }

    // Make sure that we are using system context if using mouse messages
    // to indicate the pen position.
    if (g_app->_useMouseMessages) {
        g_app->_openSystemContext = true;
    }

    SetPaintingMode();

    // InitSettings
    _window_class = std::make_unique<WindowClass>(_instance, TEXT("Mashiro"));
    _window = std::make_unique<Window>(800, 600, TEXT("Mashiro"));

    if (std::filesystem::exists("./mashiro.msh")) {
        _file = File::Open("./mashiro.msh");
        _canvas = Canvas::Open(_file.get());
        SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());
    } else {
        New();
    }
}

App::~App() noexcept {
    //_canvas->Save(_file.get());
    //_file->Save();

    UnloadWintab();
    Log::Info(TEXT("Mashiro closing"));
}

void App::Run() const {
    _window->Show();

    Log::Info(TEXT("Mashiro running"));

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

bool NEAR App::OpenTabletContexts(HWND hWnd) {

    int ctxIndex = 0;
    g_app->nOpenContexts = 0;
    g_app->nAttachedDevices = 0;
    std::stringstream szTabletName;

    g_app->_contextMap.clear();

    gpWTInfoA(WTI_INTERFACE, IFC_NDEVICES, &g_app->nAttachedDevices);
    WacomTrace("Number of attached devices: %i\n", g_app->nAttachedDevices);

    // Open/save contexts until first failure to open a context.
    // Note that gpWTInfoA(WTI_STATUS, STA_CONTEXTS, &nOpenContexts);
    // will not always let you enumerate through all contexts.
    do {
        int foundCtx = 0;
        LOGCONTEXTA lcMine = {0};
        UINT wWTInfoRetVal = 0;
        AXIS tabletX = {0};
        AXIS tabletY = {0};
        AXIS Pressure = {0};

        WacomTrace("Getting info on contextIndex: %i ...\n", ctxIndex);

        if (g_app->_openSystemContext) {
            // Opens a system context; XY returned as pixels for all
            // attached tablets.
            WacomTrace("Opening WTI_DEFSYSCTX (system context)...\n");
            foundCtx = gpWTInfoA(WTI_DEFSYSCTX, 0, &lcMine);
        } else {
            // Opens a digitizer context; XY returned as tablet coordinates for
            // each attached tablet.
            WacomTrace("Opening WTI_DDCTXS (digitizer context)...\n");
            foundCtx = gpWTInfoA(WTI_DDCTXS + ctxIndex, 0, &lcMine);

            // Use this flavor of digitizing context if not enumerating tablets.
            // Opens a "virtual" context used for all tablets.
            // foundCtx = gpWTInfoA(WTI_DEFCONTEXT, 0, &lcMine);
        }

        if (foundCtx > 0) {
            UINT result = 0;
            wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + ctxIndex, DVC_HARDWARE, &result);
            bool displayTablet = result & HWC_INTEGRATED;

            gpWTInfoA(WTI_DEVICES + ctxIndex, DVC_PKTRATE, &result);
            WacomTrace("pktrate: %i\n", result);

            char name[1024];
            gpWTInfoA(WTI_DEVICES + -1, DVC_NAME, name);
            WacomTrace("name: %s\n", name);

            WacomTrace("Current context tablet type is: %s\n", displayTablet ? "display (integrated)" : "opaque");

            lcMine.lcPktData = PACKETDATA;
            lcMine.lcOptions |= CXO_MESSAGES;

            if (g_app->_penMovesSystemCursor) {
                lcMine.lcOptions |= CXO_SYSTEM; // move system cursor
            } else {
                lcMine.lcOptions &= ~CXO_SYSTEM; // don't move system cursor
            }

            lcMine.lcPktMode = PACKETMODE;
            lcMine.lcMoveMask = PACKETDATA;
            lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;

            // Set the entire tablet as active
            wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + ctxIndex, DVC_X, &tabletX);
            if (wWTInfoRetVal != sizeof(AXIS)) {
                WacomTrace("This context should not be opened.\n");
                continue;
            }

            wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + ctxIndex, DVC_Y, &tabletY);

            gpWTInfoA(WTI_DEVICES + ctxIndex, DVC_NPRESSURE, &Pressure);
            WacomTrace("Pressure: %i, %i\n", Pressure.axMin, Pressure.axMax);

            if (g_app->_openSystemContext) {
                // leave lcIn* and lcOut* as-is except for reversing lcOutExtY.
            } else // digitizer context
            {
                // This is essential code that picks up orientation changes.
                // The reason for the calculations is to convert the tablet
                // Max/Min values to extents (counts).
                lcMine.lcOutExtX = tabletX.axMax - tabletX.axMin + 1;
                lcMine.lcOutExtY = tabletY.axMax - tabletY.axMin + 1;

                if (g_app->_useActualDigitizerOutput) {
                    // This is bumped to communicate to the driver that we
                    // want to use the fixed behavior to get actual tablet output.
                    lcMine.lcOutExtX++;
                }
            }

            // In Wintab, the tablet origin is lower left.  Move origin to upper left
            // so that it coincides with screen origin.
            lcMine.lcOutExtY = -lcMine.lcOutExtY;

            // Leave the system origin and extents as received:
            // lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

            DumpWintabContext(lcMine);

            // Open the context enabled.
            HCTX hCtx = gpWTOpenA(hWnd, &lcMine, true);

            // Save the first context, to be used to poll first tablet found when
            // mouse messages are received.
            if (g_app->_useMouseMessages && !g_app->_hCtxUsedForPolling && hCtx) {
                g_app->_hCtxUsedForPolling = hCtx;
            }

            if (hCtx) {

                TabletInfo info = {Pressure.axMax, RGB(0, 0, 0)};
                sprintf(info.name, "Tablet: %i\n", ctxIndex);
                info.tabletXExt = tabletX.axMax;
                info.tabletYExt = tabletY.axMax;
                info.displayTablet = displayTablet;
                g_app->_contextMap[hCtx] = info;
                WacomTrace("Opened context: 0x%X for ctxIndex: %i\n", hCtx, ctxIndex);
                g_app->nOpenContexts++;
            } else {
                WacomTrace("Did NOT open context for ctxIndex: %i\n", ctxIndex);
            }
        } else {
            WacomTrace("No context info for ctxIndex: %i, bailing out...\n", ctxIndex);
            break;
        }

        if (g_app->_openSystemContext) {
            break; // we're done; only the one context; bail out...
        }

        ctxIndex++;
    } while (true);

    if (g_app->nOpenContexts < g_app->nAttachedDevices && !g_app->_openSystemContext) {
        ShowError("Oops - did not open a context for each attached device");
    }

    return g_app->nAttachedDevices > 0;
}

///////////////////////////////////////////////////////////////////////////////
// Close all opened tablet contexts.
//
void App::CloseTabletContexts(void) {
    // Close all contexts we opened so we don't have them lying around in prefs.
    for (std::map<HCTX, App::TabletInfo>::iterator it = g_app->_contextMap.begin(); it != g_app->_contextMap.end();
         ++it) {
        HCTX hCtx = it->first;
        WacomTrace("Closing context: 0x%X\n", hCtx);

        if (hCtx != nullptr) {
            gpWTClose(hCtx);
        }
    }

    g_app->_contextMap.clear();

    g_app->nOpenContexts = 0;
    g_app->nAttachedDevices = 0;

    g_app->_hctx = nullptr;
    g_app->_hctxLast = nullptr;
    g_app->_hCtxUsedForPolling = nullptr;
    g_app->_scaleWidth = 0.0;
    g_app->_scaleHeight = 0.0;
}

///////////////////////////////////////////////////////////////////////////////

void App::UpdateWindowExtents(HWND hWnd) {
    // Compute scaling factor from tablet to display.
    if (g_app->_contextMap.count(g_app->_hctx) != 0) {
        App::TabletInfo info = g_app->_contextMap[g_app->_hctx];
        if (g_app->_contextMap[g_app->_hctx].displayTablet) {
            if (g_app->_kioskDisplay) {
                // Scale tablet to app window rect
                ::GetWindowRect(hWnd, &g_app->_windowRect);
                double winWidth =
                    static_cast<double>(g_app->_windowRect.right) - static_cast<double>(g_app->_windowRect.left);
                double winHeight =
                    static_cast<double>(g_app->_windowRect.bottom) - static_cast<double>(g_app->_windowRect.top);
                g_app->_scaleWidth = winWidth / (double)info.tabletXExt;
                g_app->_scaleHeight = winHeight / (double)info.tabletYExt;
            } else {
                // Scale tablet to monitor
                HMONITOR hMon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                g_app->_monInfo = {0};
                g_app->_monInfo.cbSize = sizeof(MONITORINFO);
                ::GetMonitorInfo(hMon, &g_app->_monInfo);
                double monWidth = static_cast<double>(g_app->_monInfo.rcMonitor.right) -
                                  static_cast<double>(g_app->_monInfo.rcMonitor.left);
                double monHeight = static_cast<double>(g_app->_monInfo.rcMonitor.bottom) -
                                   static_cast<double>(g_app->_monInfo.rcMonitor.top);
                g_app->_scaleWidth = monWidth / (double)info.tabletXExt;
                g_app->_scaleHeight = monHeight / (double)info.tabletYExt;

                // WacomTrace("UpdateWindowExtents: mon: %f,%f, tabletExt: %f,%f\n", monWidth, monHeight,
                // (double)info.tabletXExt, (double)info.tabletYExt);
            }
        } else {
            // Scales tablet to entire desktop.
            g_app->_scaleWidth = (double)g_app->_sysWidth / (double)info.tabletXExt;
            g_app->_scaleHeight = (double)g_app->_sysHeight / (double)info.tabletYExt;
        }

        InvalidateRect(hWnd, nullptr, true);
    }
}

///////////////////////////////////////////////////////////////////////////////

void App::UpdateSystemExtents() {
    g_app->_sysWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    g_app->_sysHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    g_app->_sysOrigX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    g_app->_sysOrigY = GetSystemMetrics(SM_YVIRTUALSCREEN);
}

///////////////////////////////////////////////////////////////////////////////

bool App::HasAttachedDisplayTablet() {
    for (std::map<HCTX, App::TabletInfo>::iterator it = g_app->_contextMap.begin(); it != g_app->_contextMap.end();
         ++it) {
        if (it->second.displayTablet) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

void App::Init(HWND hwnd) {
    UpdateSystemExtents();

    // Initialize a Wintab context for each connected tablet.
    if (!OpenTabletContexts(hwnd)) {
        throw std::runtime_error("No tablets found.");
        // SendMessage(hWnd, WM_DESTROY, 0, 0L);
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    _viewport = std::make_unique<Viewport>(glm::ivec2(800, 600));

    //_app_uniformbuffer = Uniformbuffer::Create(TEXT("App Uniformbuffer"), 0, 0, nullptr);

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
    _canvas->Render(_viewport.get());

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

static COMDLG_FILTERSPEC file_spec[] = {{TEXT("Mashiro files"), TEXT("*.msh")}, {TEXT("All files"), TEXT("*.*")}};

std::optional<std::filesystem::path> SaveAsDialog(std::filesystem::path directory, std::filesystem::path filename) {
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw new std::runtime_error("Directory provided as default path for SaveAsDialog does not exists");
    }
    directory = std::filesystem::absolute(directory);

    IFileDialog *pfd{};
    IShellItem *psiResult{};
    IShellItem *folder{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));
    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(file_spec), file_spec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetFileName(filename.wstring().c_str());
    hr = pfd->SetDefaultExtension(L"msh");
    hr = SHCreateItemFromParsingName(directory.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
    hr = pfd->SetDefaultFolder(folder);

    hr = pfd->Show(App::Get()->_window->Hwnd());
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        return {};
    }

    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    folder->Release();
    psiResult->Release();
    pfd->Release();

    return path;
}

std::optional<std::filesystem::path> OpenDialog(std::filesystem::path directory) {
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        throw new std::runtime_error("Directory provided as default path for OpenDialog does not exists");
    }

    directory = std::filesystem::absolute(directory);

    IFileDialog *pfd{};
    IShellItem *folder{};
    IShellItem *psiResult{};
    PWSTR pszFilePath{};

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    hr = pfd->SetFileTypes(ARRAYSIZE(file_spec), file_spec);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetDefaultExtension(L"msh");
    hr = SHCreateItemFromParsingName(directory.wstring().c_str(), NULL, IID_PPV_ARGS(&folder));
    hr = pfd->SetDefaultFolder(folder);

    hr = pfd->Show(App::Get()->_window->Hwnd());
    if (HRESULT_FROM_WIN32(hr) == ERROR_CANCELLED) {
        return {};
    }

    hr = pfd->GetResult(&psiResult);
    hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    const auto path = std::filesystem::path(pszFilePath);

    CoTaskMemFree(pszFilePath);
    folder->Release();
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
    const auto path = SaveAsDialog(directory, filename);
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
    if (!_file->IsSaved() || !_canvas->IsSaved()) {
        const auto result =
            MessageBox(_window->Hwnd(), std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(),
                       TEXT("Mashiro"), MB_YESNOCANCEL);
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

    _canvas.release();
    _file.release();

    const auto path = OpenDialog("./");
    if (!path.has_value()) {
        return false;
    }

    _file = File::Open(path.value());
    _canvas = Canvas::Open(_file.get());

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());
    _window->Render();
    return true;
}

bool App::New() {
    if (!_file->IsSaved() || !_canvas->IsSaved()) {
        const auto result =
            MessageBox(_window->Hwnd(), std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(),
                       TEXT("Mashiro"), MB_YESNOCANCEL);
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

    _canvas.release();
    _file.release();

    _file = File::New("unnamed.msh");
    _canvas = Canvas::Open(_file.get());

    SetWindowText(_window->Hwnd(), _file->GetDisplayName().c_str());

    _window->Render();

    return true;
}

void App::Exit() {
    if (!_file->IsSaved() || !_canvas->IsSaved()) {
        const auto result =
            MessageBox(_window->Hwnd(), std::format(TEXT("Save {}"), _file->GetFilename().wstring()).c_str(),
                       TEXT("Mashiro"), MB_YESNOCANCEL);
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

    DestroyWindow(_window->Hwnd());
}