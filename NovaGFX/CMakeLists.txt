cmake_minimum_required(VERSION 3.20)
project(NovaGFX VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NOVA_BUILD_SHARED "Build NovaGFX as a shared library" OFF)
option(NOVA_ENABLE_ASSERTS "Enable NOVA_ASSERT" ON)
option(NOVA_BUILD_EXAMPLES "Build examples" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(NOVA_ENABLE_ASSERTS OFF CACHE BOOL "Enable NOVA_ASSERT" FORCE)
endif()

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=3.3" CACHE STRING "" FORCE)
set(GLAD_GENERATOR "c" CACHE STRING "" FORCE)
set(GLAD_INSTALL OFF CACHE BOOL "" FORCE)
set(GLAD_REPRODUCIBLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glad)

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Box2D
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v2.4.1
)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(box2d)

file(GLOB_RECURSE NOVA_SOURCES "src/*.cpp")
file(GLOB_RECURSE NOVA_HEADERS "include/*.hpp")

if(NOVA_BUILD_SHARED)
    add_library(nova SHARED ${NOVA_SOURCES} ${NOVA_HEADERS})
    target_compile_definitions(nova PRIVATE NOVA_BUILD_DLL)
    target_compile_definitions(nova PUBLIC NOVA_SHARED)
else()
    add_library(nova STATIC ${NOVA_SOURCES} ${NOVA_HEADERS})
endif()

if(NOVA_ENABLE_ASSERTS)
    target_compile_definitions(nova PUBLIC NOVA_ENABLE_ASSERTS)
endif()

target_include_directories(nova
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${stb_SOURCE_DIR}
)

target_link_libraries(nova PUBLIC glfw glad box2d)

if(MSVC)
    target_compile_options(nova PRIVATE /W4 /WX /permissive- /FS)
else()
    target_compile_options(nova PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    target_link_libraries(nova PRIVATE opengl32)
endif()

if(NOVA_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_world)
    add_subdirectory(examples/physics_demo)
    add_subdirectory(examples/car_game)
    add_subdirectory(examples/neon_drift)
    add_subdirectory(examples/ui_demo)
endif()
