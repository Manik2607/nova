#include "nova/renderer/framebuffer.hpp"
#include <glad/glad.h>
#include <utility>

namespace nova {

Framebuffer::~Framebuffer() {
    destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_fbo(other.m_fbo), m_color_tex(other.m_color_tex),
      m_width(other.m_width), m_height(other.m_height)
{
    other.m_fbo = 0;
    other.m_color_tex = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        destroy();
        m_fbo = other.m_fbo;
        m_color_tex = other.m_color_tex;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_fbo = 0;
        other.m_color_tex = 0;
    }
    return *this;
}

void Framebuffer::create(i32 width, i32 height, i32 samples) {
    destroy();
    m_width = width;
    m_height = height;
    m_samples = samples;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color attachment
    glGenTextures(1, &m_color_tex);
    if (samples > 1) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_color_tex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA16F, width, height, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_color_tex, 0);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_color_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_tex, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resolve(const Framebuffer& target) const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.m_fbo);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, target.m_width, target.m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    if (m_color_tex) { glDeleteTextures(1, &m_color_tex); m_color_tex = 0; }
    if (m_fbo) { glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(i32 width, i32 height, i32 samples) {
    if (samples == -1) samples = m_samples;
    if (width == m_width && height == m_height && samples == m_samples) return;
    create(width, height, samples);
}

} // namespace nova
