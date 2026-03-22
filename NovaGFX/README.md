# NovaGFX Renderer

NovaGFX is a lightweight, high-performance 2D renderer built with C++20 and OpenGL 3.3. It's designed to be used as a static library for games and graphical applications.

## Features

- **High-Performance 2D Batch Rendering**: Efficiently draw thousands of quads, circles, and lines in a single draw call.
- **Modern C++ API**: Clean, intuitive interface using C++20 features.
- **Windowing & Input**: Built-in GLFW-based windowing and input handling (Keyboard/Mouse).
- **Math Library**: Integrated 2D math library (Vectors, Matrices, Colors).
- **Physics Integration**: Optional Box2D integration for 2D physics.
- **Asset Management**: Simple API for loading and using textures and shaders.

## Getting Started

### 1. Prerequisites

To use NovaGFX, you'll need:
- A C++20 compatible compiler (GCC 11+, Clang 13+, or MSVC 2019+).
- CMake 3.20+ (optional, but recommended).
- OpenGL 3.3+ drivers.

### 2. Header Files & Imports

All public headers are located in the `include/` directory. To use the library, include the main header:

```cpp
#include <nova/nova.hpp>
```

This header includes all core components:
- `Window` management.
- `Renderer2D`.
- `Input` handling.
- Math utilities (`Vector2f`, `Color`, etc.).
- Texture and Shader assets.

### 3. Basic Boilerplate Code

Here is a minimal example to get a window up and running with a clear color:

```cpp
#include <nova/nova.hpp>
#include <iostream>

using namespace nova;

int main() {
    try {
        // Create a 1280x720 window
        Window window(1280, 720, "My NovaGFX App");
        window.set_vsync(true);

        // Initialize the 2D renderer
        Renderer2D renderer;

        while (!window.should_close()) {
            // Process events and input
            window.poll_events();
            Input::update();

            if (Input::is_key_pressed(Key::ESCAPE)) {
                window.close();
            }

            // Clear the screen
            Vector2i size = window.get_size();
            glViewport(0, 0, size.x, size.y);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Initialize camera and start rendering
            renderer.init_camera(
                Vector2f(size.x / 2.0f, size.y / 2.0f), // Center
                1.0f,                                   // Zoom
                Vector2f(size.x, size.y)              // Viewport size
            );

            renderer.begin();
            
            // Draw a yellow rectangle at (200, 200) with size 100x100
            renderer.draw_rect(Vector2f(200, 200), Vector2f(100, 100), Color::YELLOW());
            
            renderer.end();

            // Display the frame
            window.swap_buffers();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
```

## Compilation and Linking

When using NovaGFX as a static library (`libnova.a` or `nova.lib`), you must also link against its dependencies:

- **Windows (MinGW/GCC)**:
  `-lnova -lglfw3 -lbox2d -lopengl32 -lgdi32`
- **Windows (MSVC)**:
  `nova.lib glfw3.lib box2d.lib opengl32.lib`

Ensure your include path points to the NovaGFX `include` directory and the dependencies' headers.
