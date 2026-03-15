#include "nova/assets/texture2d.hpp"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace nova {

void Texture2D::setup_gl(const u8* pixels, FilterMode filter, WrapMode wrap) {
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    i32 gl_wrap = GL_REPEAT;
    if (wrap == WrapMode::CLAMP) gl_wrap = GL_CLAMP_TO_EDGE;
    else if (wrap == WrapMode::MIRRORED_REPEAT) gl_wrap = GL_MIRRORED_REPEAT;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap);

    i32 gl_filter = (filter == FilterMode::LINEAR) ? GL_LINEAR : GL_NEAREST;
    // For simplicity, no mipmaps setup, just plain filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

    i32 format = GL_RGBA;
    if (m_channels == 1) format = GL_RED;
    else if (m_channels == 3) format = GL_RGB;
    else if (m_channels == 4) format = GL_RGBA;

    // By default stbi loads into unsigned bytes
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, pixels);
}

Texture2D::Texture2D(std::string_view path, FilterMode filter, WrapMode wrap) {
    stbi_set_flip_vertically_on_load(true);
    u8* pixels = stbi_load(path.data(), &m_width, &m_height, &m_channels, 0);
    NOVA_ASSERT(pixels != nullptr, "Failed to load texture file");

    setup_gl(pixels, filter, wrap);
    stbi_image_free(pixels);
}

Texture2D::Texture2D(const u8* pixels, i32 width, i32 height, i32 channels, FilterMode filter, WrapMode wrap)
    : m_width(width), m_height(height), m_channels(channels) {
    setup_gl(pixels, filter, wrap);
}

Texture2D::~Texture2D() {
    if (m_handle) {
        glDeleteTextures(1, &m_handle);
    }
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : m_handle(other.m_handle), m_width(other.m_width), m_height(other.m_height), m_channels(other.m_channels) {
    other.m_handle = 0;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        if (m_handle) glDeleteTextures(1, &m_handle);
        m_handle = other.m_handle;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        other.m_handle = 0;
    }
    return *this;
}

void Texture2D::bind(u32 slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

void Texture2D::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace nova
