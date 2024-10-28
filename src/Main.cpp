#include "App.h"
#include "Framework.h"
#include "Log.h"
#include <stdexcept>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                    _In_ int nShowCmd) {
    try {

        LOG_INFO(lpCmdLine);
        App app(hInstance, nShowCmd);

        if (lpCmdLine && *lpCmdLine) {
            if (std::filesystem::exists(lpCmdLine)) {
                app._file = File::Open(lpCmdLine);
                app._canvas = Canvas::Open(app._file.get());
                SetWindowText(app._window->Hwnd(), app._file->GetDisplayName().c_str());
            }
        } else {
            app.New();
        }
        app.Run();
    } catch (const std::runtime_error &e) {
        tstring err = ConvertString(e.what());
        LOG_INFO(err);
        MessageBox(nullptr, err.c_str(), TEXT("Mashiro"), MB_ICONERROR | MB_OK);
        return -1;
    }

    return 0;
}

int main() {
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_NORMAL);
}