#include "App.h"

#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int main() {
    App app;
    app.Run();

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    return main();
}
