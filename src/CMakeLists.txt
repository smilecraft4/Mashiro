# Mashiro executable

find_package(wxWidgets CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(SPDLOG_BUILD_WARNINGS OFF)
set(SPDLOG_TIDY OFF)
set(wxBUILD_PRECOMP ON)

add_executable(mashiro WIN32)

target_link_libraries(mashiro PRIVATE 
    CLI11::CLI11 
    spdlog::spdlog 
    wx::core 
    wx::base
)

target_sources(mashiro PRIVATE 
    Mashiro.h Mashiro.cpp
)

# Install

install(TARGETS mashiro
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES $<TARGET_RUNTIME_DLLS:mashiro> TYPE BIN)
