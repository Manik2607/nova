add_executable(font_test main.cpp)

target_link_libraries(font_test PRIVATE
    nova
    glad
    glfw
)
