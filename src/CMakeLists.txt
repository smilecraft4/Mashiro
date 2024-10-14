# Mashiro executable

set(SPDLOG_BUILD_WARNINGS OFF)
set(SPDLOG_TIDY OFF)
set(CMAKE_AUTORCC ON)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED) 
find_package(Stb REQUIRED)


add_executable(mashiro)
target_sources(mashiro PRIVATE 
    "Main.cpp" "Mashiro.rc"
    "App.h" "App.cpp"
    "Canvas.h" "Canvas.cpp" 
    "Viewport.h" "Viewport.cpp"
    "Brush.h" "Brush.cpp" 
    "Tile.h" "Tile.cpp" 
    "Stylus.h" "Stylus.cpp"
)

# Install
target_link_libraries(mashiro PRIVATE 
    spdlog::spdlog 
    glfw 
    glad::glad 
    glm::glm
    mashiro::data
)

target_include_directories(mashiro PRIVATE ${Stb_INCLUDE_DIR})

install(TARGETS mashiro
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES $<TARGET_RUNTIME_DLLS:mashiro> TYPE BIN)
