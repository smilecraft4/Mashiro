#include "App.h"

#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main(int argc, char *argv[]) {
    App app;
    app.Run();

    return 0;
}