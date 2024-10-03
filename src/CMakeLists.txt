# Mashiro executable

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(SPDLOG_BUILD_WARNINGS OFF)
set(SPDLOG_TIDY OFF)

add_executable(mashiro)

target_link_libraries(mashiro PRIVATE 
    CLI11::CLI11 
    spdlog::spdlog
    glad::glad
    imgui::imgui
    glfw
)

target_sources(mashiro PRIVATE 
    "Main.cpp"
    "Mashiro.h" "Mashiro.cpp"
    "Program.h" "Program.cpp"
    "Tool.h" "Tool.cpp"
    "Canvas.h" 
    "Brush.h"     
    "Tile.h"
    "Viewport.h"
    "Texture.h"
    "Coordinates.h" 
)
# Install

install(TARGETS mashiro
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES $<TARGET_RUNTIME_DLLS:mashiro> TYPE BIN)
