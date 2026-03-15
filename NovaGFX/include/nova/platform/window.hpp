#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include <string_view>
#include <string>

typedef struct GLFWwindow GLFWwindow;

namespace nova {

class Window {
public:
    Window(i32 width, i32 height, std::string_view title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void poll_events();
    void swap_buffers();
    bool should_close() const;
    void close();

    void set_vsync(bool enabled);
    void set_title(std::string_view title);
    void set_size(i32 w, i32 h);

    Vector2i get_size() const;
    Vector2i get_framebuffer_size() const;

    GLFWwindow* raw_handle() const { return m_handle; }

private:
    GLFWwindow* m_handle{nullptr};
    std::string m_title;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void joystick_callback(int jid, int event);
};

} // namespace nova
