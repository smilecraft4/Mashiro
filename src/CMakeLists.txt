# Mashiro executable

set(SPDLOG_BUILD_WARNINGS OFF)
set(SPDLOG_TIDY OFF)
set(CMAKE_AUTORCC ON)


add_executable(mashiro)
target_sources(mashiro PRIVATE 
    "Main.cpp" "Mashiro.rc"
    "App.h" "App.cpp"
    "Canvas.h" "Canvas.cpp" 
    "Viewport.h" "Viewport.cpp"
    "Brush.h" "Brush.cpp" 
    "Tile.h" "Tile.cpp" 
    "Stylus.h" "Stylus.cpp"
    "File.cpp" "File.h"
 "Settings.h" "Settings.cpp")

# Install
target_link_libraries(mashiro PRIVATE 
    spdlog::spdlog 
    glfw 
    glad::glad 
    glm::glm
    mashiro::data
    PNG::PNG
)

target_include_directories(mashiro PRIVATE ${Stb_INCLUDE_DIR})

install(TARGETS mashiro
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES $<TARGET_RUNTIME_DLLS:mashiro> TYPE BIN)
