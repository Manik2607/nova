/**
 * PostProcessPipeline — Screen-space post-processing effects.
 *
 * API Surface:
 *   PostProcessPipeline pp;
 *   pp.init(1280, 720);
 *   pp.resize(w, h);
 *   pp.begin_scene();   // binds scene FBO — render your scene into this
 *   pp.end_scene();     // unbinds scene FBO
 *   pp.render();        // runs all enabled passes and draws to default framebuffer
 *
 *   pp.set_bloom_enabled(true);
 *   pp.set_bloom_threshold(0.8f);
 *   pp.set_bloom_intensity(1.5f);
 *   pp.set_vignette_enabled(true);
 *   pp.set_vignette_intensity(0.3f);
 *   pp.set_chromatic_aberration_enabled(true);
 *   pp.set_chromatic_aberration_strength(0.005f);
 */
#pragma once
#include "nova/core.hpp"
#include "nova/renderer/framebuffer.hpp"
#include "nova/assets/shader.hpp"
#include <memory>

namespace nova {

class PostProcessPipeline {
public:
    PostProcessPipeline() = default;
    ~PostProcessPipeline() = default;

    void init(i32 width, i32 height);
    void resize(i32 width, i32 height);

    /// Call before rendering the scene — redirects to internal FBO.
    void begin_scene();
    /// Call after rendering the scene.
    void end_scene();
    /// Run all post-process passes and output to the default framebuffer.
    void render();

    // Bloom
    void set_bloom_enabled(bool v) { m_bloom_enabled = v; }
    void set_bloom_threshold(f32 v) { m_bloom_threshold = v; }
    void set_bloom_intensity(f32 v) { m_bloom_intensity = v; }

    // Vignette
    void set_vignette_enabled(bool v) { m_vignette_enabled = v; }
    void set_vignette_intensity(f32 v) { m_vignette_intensity = v; }
    void set_vignette_radius(f32 v) { m_vignette_radius = v; }

    // Chromatic aberration
    void set_chromatic_aberration_enabled(bool v) { m_chroma_enabled = v; }
    void set_chromatic_aberration_strength(f32 v) { m_chroma_strength = v; }

    bool is_initialized() const { return m_initialized; }

private:
    void init_fullscreen_quad();
    void draw_fullscreen_quad();

    bool m_initialized = false;
    i32 m_width = 0, m_height = 0;

    // FBOs: scene, bloom ping-pong, final
    Framebuffer m_scene_fbo;
    Framebuffer m_bloom_fbo[2]; // ping-pong for blur
    Framebuffer m_final_fbo;

    // Shaders
    std::unique_ptr<Shader> m_bloom_extract_shader;
    std::unique_ptr<Shader> m_bloom_blur_shader;
    std::unique_ptr<Shader> m_composite_shader;

    // Fullscreen quad
    u32 m_quad_vao = 0;
    u32 m_quad_vbo = 0;

    // Settings
    bool m_bloom_enabled = true;
    f32 m_bloom_threshold = 0.7f;
    f32 m_bloom_intensity = 1.2f;

    bool m_vignette_enabled = true;
    f32 m_vignette_intensity = 0.4f;
    f32 m_vignette_radius = 0.75f;

    bool m_chroma_enabled = true;
    f32 m_chroma_strength = 0.003f;
};

} // namespace nova
