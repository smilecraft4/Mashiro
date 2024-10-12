cmake_minimum_required(VERSION 3.21)

project(mashiro VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)

# install(DIRECTORY data DESTINATION bin)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
