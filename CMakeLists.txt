cmake_minimum_required(VERSION 3.21)

project(mashiro VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED) 
find_package(Stb REQUIRED)
find_package(PNG REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(CMakeRC CONFIG REQUIRED)

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)

# install(DIRECTORY data DESTINATION bin)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
