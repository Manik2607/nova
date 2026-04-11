#include "nova/renderer/post_process.hpp"
#include <glad/glad.h>

namespace nova {

// ── Shader sources ──────────────────────────────────────────────────────

static const char* FULLSCREEN_VERT = R"(
#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";

static const char* BLOOM_EXTRACT_FRAG = R"(
#version 330 core
out vec4 FragColor;
in vec2 v_TexCoord;

uniform sampler2D u_Scene;
uniform float u_Threshold;

void main() {
    vec3 color = texture(u_Scene, v_TexCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > u_Threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)";

static const char* BLOOM_BLUR_FRAG = R"(
#version 330 core
out vec4 FragColor;
in vec2 v_TexCoord;

uniform sampler2D u_Image;
uniform bool u_Horizontal;
uniform float u_TexelSize;

const float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main() {
    vec3 result = texture(u_Image, v_TexCoord).rgb * weight[0];
    if (u_Horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_Image, v_TexCoord + vec2(u_TexelSize * float(i), 0.0)).rgb * weight[i];
            result += texture(u_Image, v_TexCoord - vec2(u_TexelSize * float(i), 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(u_Image, v_TexCoord + vec2(0.0, u_TexelSize * float(i))).rgb * weight[i];
            result += texture(u_Image, v_TexCoord - vec2(0.0, u_TexelSize * float(i))).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
)";

static const char* COMPOSITE_FRAG = R"(
#version 330 core
out vec4 FragColor;
in vec2 v_TexCoord;

uniform sampler2D u_Scene;
uniform sampler2D u_Bloom;

uniform bool u_BloomEnabled;
uniform float u_BloomIntensity;

uniform bool u_VignetteEnabled;
uniform float u_VignetteIntensity;
uniform float u_VignetteRadius;

uniform bool u_ChromaEnabled;
uniform float u_ChromaStrength;

void main() {
    vec2 uv = v_TexCoord;
    vec3 color;

    // Chromatic aberration
    if (u_ChromaEnabled) {
        vec2 dir = uv - vec2(0.5);
        float dist = length(dir);
        vec2 offset = dir * dist * u_ChromaStrength;
        color.r = texture(u_Scene, uv + offset).r;
        color.g = texture(u_Scene, uv).g;
        color.b = texture(u_Scene, uv - offset).b;
    } else {
        color = texture(u_Scene, uv).rgb;
    }

    // Bloom composite (additive / screen blend)
    if (u_BloomEnabled) {
        vec3 bloom = texture(u_Bloom, uv).rgb * u_BloomIntensity;
        color = vec3(1.0) - (vec3(1.0) - color) * (vec3(1.0) - bloom); // screen blend
    }

    // Vignette
    if (u_VignetteEnabled) {
        vec2 vc = uv - vec2(0.5);
        float vfactor = 1.0 - smoothstep(u_VignetteRadius, u_VignetteRadius + 0.4, length(vc));
        color *= mix(1.0, vfactor, u_VignetteIntensity);
    }

    // Tone mapping (simple Reinhard)
    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
}
)";

static const char* FXAA_FRAG = R"(
#version 330 core
out vec4 FragColor;
in vec2 v_TexCoord;

uniform sampler2D u_Scene;
uniform vec2 u_TexelSize;

// FXAA Settings
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0

void main() {
    vec3 rgbNW = texture(u_Scene, v_TexCoord + vec2(-1.0, -1.0) * u_TexelSize).rgb;
    vec3 rgbNE = texture(u_Scene, v_TexCoord + vec2(1.0, -1.0) * u_TexelSize).rgb;
    vec3 rgbSW = texture(u_Scene, v_TexCoord + vec2(-1.0, 1.0) * u_TexelSize).rgb;
    vec3 rgbSE = texture(u_Scene, v_TexCoord + vec2(1.0, 1.0) * u_TexelSize).rgb;
    vec3 rgbM  = texture(u_Scene, v_TexCoord).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * u_TexelSize;

    vec3 rgbA = 0.5 * (
        texture(u_Scene, v_TexCoord + dir * (1.0/3.0 - 0.5)).rgb +
        texture(u_Scene, v_TexCoord + dir * (2.0/3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(u_Scene, v_TexCoord + dir * -0.5).rgb +
        texture(u_Scene, v_TexCoord + dir * 0.5).rgb);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        FragColor = vec4(rgbA, 1.0);
    } else {
        FragColor = vec4(rgbB, 1.0);
    }
}
)";

// ── Implementation ──────────────────────────────────────────────────────

void PostProcessPipeline::init(i32 width, i32 height) {
    m_width = width;
    m_height = height;

    update_fbos();

    m_bloom_extract_shader = std::make_unique<Shader>(FULLSCREEN_VERT, BLOOM_EXTRACT_FRAG);
    m_bloom_blur_shader = std::make_unique<Shader>(FULLSCREEN_VERT, BLOOM_BLUR_FRAG);
    m_composite_shader = std::make_unique<Shader>(FULLSCREEN_VERT, COMPOSITE_FRAG);
    m_fxaa_shader = std::make_unique<Shader>(FULLSCREEN_VERT, FXAA_FRAG);

    init_fullscreen_quad();
    m_initialized = true;
}

void PostProcessPipeline::resize(i32 width, i32 height) {
    if (width == m_width && height == m_height) return;
    m_width = width;
    m_height = height;
    update_fbos();
}

void PostProcessPipeline::update_fbos() {
    i32 samples = (m_aa_mode == AntiAliasingMode::MSAA) ? m_msaa_samples : 1;
    m_scene_fbo.create(m_width, m_height, samples);
    m_resolve_fbo.create(m_width, m_height, 1);
    m_bloom_fbo[0].create(m_width / 2, m_height / 2, 1);
    m_bloom_fbo[1].create(m_width / 2, m_height / 2, 1);
    m_final_fbo.create(m_width, m_height, 1);
}

void PostProcessPipeline::begin_scene() {
    m_scene_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);
}

void PostProcessPipeline::end_scene() {
    m_scene_fbo.unbind();
    if (m_aa_mode == AntiAliasingMode::MSAA) {
        m_scene_fbo.resolve(m_resolve_fbo);
    }
}

void PostProcessPipeline::render() {
    u32 scene_tex = (m_aa_mode == AntiAliasingMode::MSAA) 
        ? m_resolve_fbo.get_color_texture() 
        : m_scene_fbo.get_color_texture();
    
    u32 bloom_tex = 0;

    // Bloom pass
    if (m_bloom_enabled) {
        // ... (keep bloom same as before but use correct scene_tex)
        m_bloom_fbo[0].bind();
        glClear(GL_COLOR_BUFFER_BIT);
        m_bloom_extract_shader->bind();
        m_bloom_extract_shader->set_int("u_Scene", 0);
        m_bloom_extract_shader->set_float("u_Threshold", m_bloom_threshold);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_tex);
        draw_fullscreen_quad();
        m_bloom_extract_shader->unbind();

        bool horizontal = true;
        for (int i = 0; i < 4; i++) {
            int dst = horizontal ? 1 : 0;
            int src = horizontal ? 0 : 1;
            m_bloom_fbo[dst].bind();
            glClear(GL_COLOR_BUFFER_BIT);
            m_bloom_blur_shader->bind();
            m_bloom_blur_shader->set_int("u_Image", 0);
            m_bloom_blur_shader->set_bool("u_Horizontal", horizontal);
            f32 texel_size = horizontal
                ? 1.0f / static_cast<f32>(m_bloom_fbo[src].get_width())
                : 1.0f / static_cast<f32>(m_bloom_fbo[src].get_height());
            m_bloom_blur_shader->set_float("u_TexelSize", texel_size);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_bloom_fbo[src].get_color_texture());
            draw_fullscreen_quad();
            m_bloom_blur_shader->unbind();
            horizontal = !horizontal;
        }
        bloom_tex = m_bloom_fbo[0].get_color_texture();
    }

    // Determine target for composite
    bool use_fxaa = (m_aa_mode == AntiAliasingMode::FXAA);
    if (use_fxaa) {
        m_final_fbo.bind();
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    // Composite pass
    m_composite_shader->bind();
    m_composite_shader->set_int("u_Scene", 0);
    m_composite_shader->set_int("u_Bloom", 1);
    m_composite_shader->set_bool("u_BloomEnabled", m_bloom_enabled);
    m_composite_shader->set_float("u_BloomIntensity", m_bloom_intensity);
    m_composite_shader->set_bool("u_VignetteEnabled", m_vignette_enabled);
    m_composite_shader->set_float("u_VignetteIntensity", m_vignette_intensity);
    m_composite_shader->set_float("u_VignetteRadius", m_vignette_radius);
    m_composite_shader->set_bool("u_ChromaEnabled", m_chroma_enabled);
    m_composite_shader->set_float("u_ChromaStrength", m_chroma_strength);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_tex);

    draw_fullscreen_quad();
    m_composite_shader->unbind();

    // FXAA pass if enabled
    if (use_fxaa) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT);

        m_fxaa_shader->bind();
        m_fxaa_shader->set_int("u_Scene", 0);
        m_fxaa_shader->set_vec2("u_TexelSize", {1.0f / m_width, 1.0f / m_height});
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_final_fbo.get_color_texture());
        draw_fullscreen_quad();
        m_fxaa_shader->unbind();
    }
}

void PostProcessPipeline::set_aa_mode(AntiAliasingMode mode) {
    if (m_aa_mode == mode) return;
    m_aa_mode = mode;
    if (m_initialized) update_fbos();
}

void PostProcessPipeline::set_msaa_samples(i32 samples) {
    if (m_msaa_samples == samples) return;
    m_msaa_samples = samples;
    if (m_initialized && m_aa_mode == AntiAliasingMode::MSAA) update_fbos();
}

void PostProcessPipeline::init_fullscreen_quad() {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_quad_vao);
    glGenBuffers(1, &m_quad_vbo);
    glBindVertexArray(m_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void PostProcessPipeline::draw_fullscreen_quad() {
    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

} // namespace nova
