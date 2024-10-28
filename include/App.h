#pragma once
#include <memory>

#include "Brush.h"
#include "Canvas.h"
#include "File.h"
#include "Framework.h"
#include "Inputs.h"
#include "Preferences.h"
#include "Renderer.h"
#include "Viewport.h"
#include "Window.h"

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

    void PollForPenData(HCTX hCtx_I, HWND hWnd_I, POINT &ptOld_I, UINT &prsOld_I, POINT &ptNew_O, UINT &prsNew_O);
    void CloseTabletContexts(void);
    void UpdateWindowExtents(HWND hWnd);
    bool NEAR OpenTabletContexts(HWND hWnd);
    void UpdateSystemExtents();

    bool HasAttachedDisplayTablet();

    App(HINSTANCE instance, int show_cmd);
    ~App() noexcept;

    void Run() const;

    bool Save();
    bool SaveAs();
    bool Open();
    bool New();
    void Exit();

    void EnableBrush(bool enable);

    void Init(HWND hwnd);
    void Update();
    void Render();
    void Refresh();

    void SetNavigationMode();

    void SetPaintingMode();

  public:
    HINSTANCE _instance;
    int _show_cmd;
    bool _brush_enabled;

    std::vector<tstring> _args;
    // TODO: combine into simple app args
    std::unique_ptr<WindowClass> _window_class;
    std::unique_ptr<Window> _window;

    std::unique_ptr<Preferences> _preferences;

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


    std::unique_ptr<Inputs> _inputs;
    // TODO: Create a class to handle Wintab as well as the mouse (this can a simple Stylus class that has a defined set
    // of response that has parity on mouth and on pentablet)
    typedef struct {
        int maxPressure;
        COLORREF penColor;
        char name[32];
        LONG tabletXExt;
        LONG tabletYExt;
        bool displayTablet;
    } TabletInfo;

    bool _openSystemContext = true;
    bool _useMouseMessages = true;
    HCTX _hCtxUsedForPolling = nullptr;
    bool _penMovesSystemCursor = true;
    bool _drawLines = true;
    bool _pressure = true;
    bool _offsetMode = false;
    bool _useActualDigitizerOutput = false;
    bool _kioskDisplay = false;
    double _sysWidth = 0;
    double _sysHeight = 0;
    double _sysOrigX = 0;
    double _sysOrigY = 0;

    static HCTX _hctx;
    static HCTX _hctxLast;
    static double _scaleWidth;
    static double _scaleHeight;
    static RECT _clientRect;
    static RECT _windowRect;
    static MONITORINFO _monInfo;
    const char *pszProgramName = "ScribbleDemo";
    static int nOpenContexts;
    static int nAttachedDevices;
    std::map<HCTX, TabletInfo> _contextMap;
};
