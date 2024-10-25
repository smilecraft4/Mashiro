include(Catch)

add_executable(mashiro-test
    Dummy.cpp
    Prompts.cpp
)

target_compile_definitions(mashiro-test PRIVATE _UNICODE UNICODE)
target_link_libraries(mashiro-test PRIVATE 
    Catch2::Catch2 
    Catch2::Catch2WithMain
)

catch_discover_tests(mashiro-test)