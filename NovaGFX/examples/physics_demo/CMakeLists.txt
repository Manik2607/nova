add_executable(physics_demo main.cpp)

target_link_libraries(physics_demo PRIVATE nova)

if(MSVC)
    target_compile_options(physics_demo PRIVATE /W4)
else()
    target_compile_options(physics_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()
