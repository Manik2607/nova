#pragma once
#include "nova/core.hpp"
#include "nova/math/vector2.hpp"
#include "nova/math/vector3.hpp"
#include "nova/math/vector4.hpp"
#include "nova/math/color.hpp"
#include "nova/math/mat4.hpp"
#include <string_view>
#include <string>
#include <unordered_map>

namespace nova {

class Shader {
public:
    Shader(std::string_view vert_src, std::string_view frag_src);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    static Shader from_files(std::string_view vert_path, std::string_view frag_path);

    void bind() const;
    void unbind() const;

    void set_int(std::string_view name, i32 value);
    void set_float(std::string_view name, f32 value);
    void set_vec2(std::string_view name, const Vector2f& value);
    void set_vec3(std::string_view name, const Vector3f& value);
    void set_vec4(std::string_view name, const Vector4f& value);
    void set_color(std::string_view name, const Color& value);
    void set_mat4(std::string_view name, const Mat4& value);
    void set_bool(std::string_view name, bool value);

    u32 handle() const { return m_handle; }

private:
    u32 m_handle{0};
    mutable std::unordered_map<std::string, i32> m_uniform_cache;

    i32 get_uniform_location(std::string_view name) const;
    static u32 compile_stage(std::string_view source, u32 type);
};

} // namespace nova
