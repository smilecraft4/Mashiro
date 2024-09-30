# Mashiro tests

find_package(Catch2 3 REQUIRED)

include(CTest)
include(Catch)

add_executable(mashiro-test)

target_link_libraries(mashiro-test PRIVATE 
    Catch2::Catch2WithMain
)

target_sources(mashiro-test PRIVATE 
    test-dummy.cpp
)

catch_discover_tests(mashiro-test)