#include "pch.h"

#include "App.h"
#include "Log.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                    _In_ int nShowCmd) {
    App app(hInstance, lpCmdLine, nShowCmd);
    return app.Run();
}

int main() {
    return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_NORMAL);
}