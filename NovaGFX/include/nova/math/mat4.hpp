#pragma once
#include "nova/math/vector3.hpp"
#include "nova/math/vector4.hpp"

namespace nova {

struct Mat4 {
    f32 data[16]{0};

    constexpr Mat4() noexcept = default;

    static constexpr Mat4 zero() noexcept { return Mat4(); }
    
    static constexpr Mat4 identity() noexcept {
        Mat4 m;
        m.data[0] = 1.0f; m.data[5] = 1.0f; m.data[10] = 1.0f; m.data[15] = 1.0f;
        return m;
    }

    static Mat4 perspective(f32 fov_rad, f32 aspect, f32 near_val, f32 far_val) noexcept {
        Mat4 m;
        f32 tan_half_fov = std::tan(fov_rad / 2.0f);
        m.data[0] = 1.0f / (aspect * tan_half_fov);
        m.data[5] = 1.0f / tan_half_fov;
        m.data[10] = -(far_val + near_val) / (far_val - near_val);
        m.data[11] = -1.0f;
        m.data[14] = -(2.0f * far_val * near_val) / (far_val - near_val);
        return m;
    }

    static constexpr Mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_val, f32 far_val) noexcept {
        Mat4 m = identity();
        m.data[0] = 2.0f / (right - left);
        m.data[5] = 2.0f / (top - bottom);
        m.data[10] = -2.0f / (far_val - near_val);
        m.data[12] = -(right + left) / (right - left);
        m.data[13] = -(top + bottom) / (top - bottom);
        m.data[14] = -(far_val + near_val) / (far_val - near_val);
        return m;
    }

    static constexpr Mat4 translate(const Vector3f& v) noexcept {
        Mat4 m = identity();
        m.data[12] = v.x;
        m.data[13] = v.y;
        m.data[14] = v.z;
        return m;
    }

    static constexpr Mat4 scale(const Vector3f& v) noexcept {
        Mat4 m = identity();
        m.data[0] = v.x;
        m.data[5] = v.y;
        m.data[10] = v.z;
        return m;
    }

    static Mat4 rotate_x(f32 angle_rad) noexcept {
        Mat4 m = identity();
        f32 c = std::cos(angle_rad), s = std::sin(angle_rad);
        m.data[5] = c; m.data[6] = s;
        m.data[9] = -s; m.data[10] = c;
        return m;
    }

    static Mat4 rotate_y(f32 angle_rad) noexcept {
        Mat4 m = identity();
        f32 c = std::cos(angle_rad), s = std::sin(angle_rad);
        m.data[0] = c; m.data[2] = -s;
        m.data[8] = s; m.data[10] = c;
        return m;
    }

    static Mat4 rotate_z(f32 angle_rad) noexcept {
        Mat4 m = identity();
        f32 c = std::cos(angle_rad), s = std::sin(angle_rad);
        m.data[0] = c; m.data[1] = s;
        m.data[4] = -s; m.data[5] = c;
        return m;
    }

    static Mat4 look_at(const Vector3f& eye, const Vector3f& center, const Vector3f& up) noexcept {
        Vector3f f = (center - eye).normalized();
        Vector3f s = f.cross(up).normalized();
        Vector3f u = s.cross(f);

        Mat4 m = identity();
        m.data[0] = s.x; m.data[4] = s.y; m.data[8] = s.z;
        m.data[1] = u.x; m.data[5] = u.y; m.data[9] = u.z;
        m.data[2] = -f.x; m.data[6] = -f.y; m.data[10] = -f.z;
        m.data[12] = -s.dot(eye);
        m.data[13] = -u.dot(eye);
        m.data[14] = f.dot(eye);
        return m;
    }

    constexpr Mat4 operator*(const Mat4& o) const noexcept {
        Mat4 r;
        for (int c = 0; c < 4; ++c) {
            for (int r_idx = 0; r_idx < 4; ++r_idx) {
                r.data[c * 4 + r_idx] = 
                    data[0 * 4 + r_idx] * o.data[c * 4 + 0] +
                    data[1 * 4 + r_idx] * o.data[c * 4 + 1] +
                    data[2 * 4 + r_idx] * o.data[c * 4 + 2] +
                    data[3 * 4 + r_idx] * o.data[c * 4 + 3];
            }
        }
        return r;
    }

    constexpr Vector4f operator*(const Vector4f& v) const noexcept {
        return {
            data[0]*v.x + data[4]*v.y + data[8]*v.z + data[12]*v.w,
            data[1]*v.x + data[5]*v.y + data[9]*v.z + data[13]*v.w,
            data[2]*v.x + data[6]*v.y + data[10]*v.z + data[14]*v.w,
            data[3]*v.x + data[7]*v.y + data[11]*v.z + data[15]*v.w
        };
    }

    constexpr const f32* ptr() const noexcept { return data; }

    constexpr Mat4 transposed() const noexcept {
        Mat4 r;
        for (int c = 0; c < 4; ++c) {
            for (int r_idx = 0; r_idx < 4; ++r_idx) {
                r.data[c * 4 + r_idx] = data[r_idx * 4 + c];
            }
        }
        return r;
    }

    Mat4 inversed() const noexcept {
        Mat4 inv;
        f32* out = inv.data;
        const f32* m = data;

        out[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] + 
                 m[9]  * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];
        out[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] - 
                  m[8]  * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];
        out[8] = m[4]  * m[9] * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5] * m[15] + 
                 m[8]  * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        out[12] = -m[4]  * m[9] * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5] * m[14] - 
                   m[8]  * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        out[1] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2] * m[15] - 
                  m[9]  * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        out[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2] * m[15] + 
                 m[8]  * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        out[9] = -m[0]  * m[9] * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1] * m[15] - 
                  m[8]  * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        out[13] = m[0]  * m[9] * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1] * m[14] + 
                  m[8]  * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        out[2] = m[1]  * m[6] * m[15] - m[1]  * m[7] * m[14] - m[5]  * m[2] * m[15] + 
                 m[5]  * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        out[6] = -m[0]  * m[6] * m[15] + m[0]  * m[7] * m[14] + m[4]  * m[2] * m[15] - 
                  m[4]  * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        out[10] = m[0]  * m[5] * m[15] - m[0]  * m[7] * m[13] - m[4]  * m[1] * m[15] + 
                  m[4]  * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        out[14] = -m[0]  * m[5] * m[14] + m[0]  * m[6] * m[13] + m[4]  * m[1] * m[14] - 
                   m[4]  * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        out[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - 
                  m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        out[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + 
                 m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        out[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - 
                   m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        out[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + 
                  m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        f32 det = m[0] * out[0] + m[1] * out[4] + m[2] * out[8] + m[3] * out[12];
        if (det != 0.0f) {
            f32 inv_det = 1.0f / det;
            for (int i = 0; i < 16; ++i) { out[i] *= inv_det; }
        }
        return inv;
    }
};

} // namespace nova
