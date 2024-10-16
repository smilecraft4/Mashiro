# Mashiro tests

include(CTest)
include(Catch)

find_package(WebP CONFIG REQUIRED)

add_executable(mashiro-test)

target_link_libraries(mashiro-test PRIVATE 
    Catch2::Catch2 
    Catch2::Catch2WithMain
    glfw
    glad::glad
    PNG::PNG
    WebP::webp WebP::webpdecoder WebP::webpdemux
)

target_sources(mashiro-test PRIVATE 
    # "test-libpng.cpp"
    # "test-libwebp.cpp"
    # "test-fileformat.cpp"
 "test-png_reader.cpp")

catch_discover_tests(mashiro-test)