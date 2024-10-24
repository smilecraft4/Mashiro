cmake_minimum_required(VERSION 3.21)

project(mashiro 
    VERSION 0.0.1 
    DESCRIPTION "Mashiro is a simple infinite canvas raster painting app"
    HOMEPAGE_URL "https://github.com/smilecraft4/Mashiro"
    LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 23)  

find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED) 
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Catch2 3 REQUIRED)


message(STATUS "LibPNG package was found with include dir: '${PNG_LIBRARIES}'")

add_executable(mashiro WIN32
    src/AABB.cpp
    src/App.cpp
    src/Brush.cpp
    src/Canvas.cpp  
    src/File.cpp
    src/Framework.cpp
    src/Log.cpp
    src/Main.cpp
    src/Mashiro.rc
    src/Preferences.cpp
    src/Renderer.cpp
    src/Viewport.cpp
    src/Viewport.cpp
    src/Window.cpp
)

target_compile_definitions(mashiro PRIVATE _UNICODE UNICODE)

# Install
target_link_libraries(mashiro PRIVATE 
    glad::glad 
    glm::glm
    PNG::PNG
)

target_include_directories(mashiro PRIVATE include/)

include(CTest)
add_subdirectory(tests)

# Define a list of files in the data directory
file(GLOB_RECURSE DATA_FILES "${CMAKE_SOURCE_DIR}/data/*")

# Custom command to copy data folder to build directory
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data" "${CMAKE_BINARY_DIR}/data"
    DEPENDS ${DATA_FILES}
    COMMENT "Copying data folder to build directory..."
)

# Add a target to ensure that the command runs as part of the build
add_custom_target(copy_data ALL DEPENDS "${CMAKE_BINARY_DIR}/data")

install(TARGETS mashiro DESTINATION bin)
install(DIRECTORY data DESTINATION bin)
install(FILES $<TARGET_RUNTIME_DLLS:mashiro> TYPE BIN)
install(TARGETS mashiro
    COMPONENT mashiro
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    DIRECTORIES $<TARGET_FILE_DIR:mashiro>)

set(PACKAGE_FILE_NAME "mashiro_${PROJECT_VERSION}_setup")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PACKAGE_FILE_NAME "mashiro_${PROJECT_VERSION}d_setup")
endif()

set(CPACK_PACKAGE_DESCRIPTION "A minimalistic infinite canvas raster painting app")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Mashiro")  # Install directory name without version
set(CPACK_PACKAGE_NAME "Mashiro")
set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")
set(CPACK_PACKAGE_VENDOR "smilecraft00")
set(CPACK_PACKAGE_CONTACT "https://x.com/smilecraft00")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)