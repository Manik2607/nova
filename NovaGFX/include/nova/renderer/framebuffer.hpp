/**
 * Framebuffer — Engine-level FBO wrapper for render-to-texture.
 *
 * API Surface:
 *   Framebuffer fbo;
 *   fbo.create(1280, 720);
 *   fbo.bind();          // render to this FBO
 *   fbo.unbind();        // back to default framebuffer
 *   fbo.resize(w, h);    // recreate at new size
 *   u32 tex = fbo.get_color_texture(); // GL texture handle
 */
#pragma once
#include "nova/core.hpp"

namespace nova {

class Framebuffer {
public:
    Framebuffer() = default;
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void create(i32 width, i32 height, i32 samples = 1);
    void destroy();
    void bind() const;
    void unbind() const;
    void resize(i32 width, i32 height, i32 samples = -1);

    /// Resolve this framebuffer into another one. Used for MSAA -> Regular texture.
    void resolve(const Framebuffer& target) const;

    u32 get_color_texture() const { return m_color_tex; }
    i32 get_width() const { return m_width; }
    i32 get_height() const { return m_height; }
    i32 get_samples() const { return m_samples; }
    bool is_valid() const { return m_fbo != 0; }
    bool is_multisampled() const { return m_samples > 1; }

private:
    u32 m_fbo{0};
    u32 m_color_tex{0};
    i32 m_width{0};
    i32 m_height{0};
    i32 m_samples{1};
};

} // namespace nova
