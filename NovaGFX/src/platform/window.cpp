#include "nova/platform/window.hpp"
#include "nova/input/input.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace nova {

Window::Window(i32 width, i32 height, std::string_view title) : m_title(title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_handle = glfwCreateWindow(width, height, m_title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glfwSetWindowUserPointer(m_handle, this);

    glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);
    glfwSetKeyCallback(m_handle, key_callback);
    glfwSetMouseButtonCallback(m_handle, mouse_button_callback);
    glfwSetCursorPosCallback(m_handle, cursor_pos_callback);
    glfwSetScrollCallback(m_handle, scroll_callback);
    glfwSetJoystickCallback(joystick_callback);

    set_vsync(true);

    int fb_w, fb_h;
    glfwGetFramebufferSize(m_handle, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
    glfwTerminate(); // Safe strictly if window is the last object. Note: multi-window apps shouldn't do this here, but 1 window is fine.
}

Window::Window(Window&& other) noexcept : m_handle(other.m_handle), m_title(std::move(other.m_title)) {
    other.m_handle = nullptr;
    if (m_handle) {
        glfwSetWindowUserPointer(m_handle, this);
    }
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (m_handle) {
            glfwDestroyWindow(m_handle);
        }
        m_handle = other.m_handle;
        m_title = std::move(other.m_title);
        other.m_handle = nullptr;
        if (m_handle) {
            glfwSetWindowUserPointer(m_handle, this);
        }
    }
    return *this;
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::swap_buffers() {
    if (m_handle) {
        glfwSwapBuffers(m_handle);
    }
}

bool Window::should_close() const {
    return m_handle && glfwWindowShouldClose(m_handle);
}

void Window::close() {
    if (m_handle) {
        glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
    }
}

void Window::set_vsync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::set_title(std::string_view title) {
    m_title = title;
    if (m_handle) {
        glfwSetWindowTitle(m_handle, m_title.c_str());
    }
}

void Window::set_size(i32 w, i32 h) {
    if (m_handle) glfwSetWindowSize(m_handle, w, h);
}

Vector2i Window::get_size() const {
    int w = 0, h = 0;
    if (m_handle) glfwGetWindowSize(m_handle, &w, &h);
    return {w, h};
}

Vector2i Window::get_framebuffer_size() const {
    int w = 0, h = 0;
    if (m_handle) glfwGetFramebufferSize(m_handle, &w, &h);
    return {w, h};
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window; (void)scancode; (void)mods;
    Input::internal_key_callback(key, action);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window; (void)mods;
    Input::internal_mouse_button_callback(button, action);
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    Input::internal_cursor_pos_callback(static_cast<f32>(xpos), static_cast<f32>(ypos));
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    Input::internal_scroll_callback(static_cast<f32>(xoffset), static_cast<f32>(yoffset));
}

void Window::joystick_callback(int jid, int event) {
    Input::internal_joystick_callback(jid, event);
}

} // namespace nova
