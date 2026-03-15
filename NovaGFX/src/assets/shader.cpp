#include "nova/assets/shader.hpp"
#include <glad/glad.h>
#include <fstream>

namespace nova {

u32 Shader::compile_stage(std::string_view source, u32 type) {
    u32 shader = glCreateShader(type);
    const char* src = source.data();
    i32 len = static_cast<i32>(source.length());
    glShaderSource(shader, 1, &src, &len);
    glCompileShader(shader);

    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info_log);
        std::string err = std::string(type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") + " shader compilation failed:\n" + info_log;
        NOVA_ASSERT(false, err.c_str());
    }
    return shader;
}

Shader::Shader(std::string_view vert_src, std::string_view frag_src) {
    u32 vert = compile_stage(vert_src, GL_VERTEX_SHADER);
    u32 frag = compile_stage(frag_src, GL_FRAGMENT_SHADER);

    m_handle = glCreateProgram();
    glAttachShader(m_handle, vert);
    glAttachShader(m_handle, frag);
    glLinkProgram(m_handle);

    i32 success;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetProgramInfoLog(m_handle, 1024, nullptr, info_log);
        std::string err = std::string("Shader program linking failed:\n") + info_log;
        NOVA_ASSERT(false, err.c_str());
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (m_handle) {
        glDeleteProgram(m_handle);
    }
}

Shader::Shader(Shader&& other) noexcept : m_handle(other.m_handle), m_uniform_cache(std::move(other.m_uniform_cache)) {
    other.m_handle = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_handle) glDeleteProgram(m_handle);
        m_handle = other.m_handle;
        m_uniform_cache = std::move(other.m_uniform_cache);
        other.m_handle = 0;
    }
    return *this;
}

Shader Shader::from_files(std::string_view vert_path, std::string_view frag_path) {
    auto read_file = [](std::string_view path) {
        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);
        NOVA_ASSERT(file.is_open(), "Failed to open shader file");
        usize size = static_cast<usize>(file.tellg());
        std::string buffer(size, '\0');
        file.seekg(0);
        file.read(buffer.data(), size);
        return buffer;
    };
    return Shader(read_file(vert_path), read_file(frag_path));
}

void Shader::bind() const { glUseProgram(m_handle); }
void Shader::unbind() const { glUseProgram(0); }

i32 Shader::get_uniform_location(std::string_view name) const {
    std::string name_str(name);
    if (auto it = m_uniform_cache.find(name_str); it != m_uniform_cache.end()) {
        return it->second;
    }
    i32 loc = glGetUniformLocation(m_handle, name.data());
    m_uniform_cache[name_str] = loc;
    return loc;
}

void Shader::set_int(std::string_view name, i32 value) { glUniform1i(get_uniform_location(name), value); }
void Shader::set_float(std::string_view name, f32 value) { glUniform1f(get_uniform_location(name), value); }
void Shader::set_vec2(std::string_view name, const Vector2f& value) { glUniform2f(get_uniform_location(name), value.x, value.y); }
void Shader::set_vec3(std::string_view name, const Vector3f& value) { glUniform3f(get_uniform_location(name), value.x, value.y, value.z); }
void Shader::set_vec4(std::string_view name, const Vector4f& value) { glUniform4f(get_uniform_location(name), value.x, value.y, value.z, value.w); }
void Shader::set_color(std::string_view name, const Color& value) { glUniform4f(get_uniform_location(name), value.r, value.g, value.b, value.a); }
void Shader::set_mat4(std::string_view name, const Mat4& value) { glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, value.ptr()); }
void Shader::set_bool(std::string_view name, bool value) { glUniform1i(get_uniform_location(name), value ? 1 : 0); }

} // namespace nova
