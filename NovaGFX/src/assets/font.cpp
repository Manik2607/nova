#include "nova/assets/font.hpp"
#include <glad/glad.h>
#include <vector>
#include <fstream>
#include <iostream>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace nova {

Font::Font(std::string_view path, f32 size) {
    std::cout << "[FONT] Opening file: " << path << std::endl;
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::cout << "[FONT] File size: " << file_size << " bytes." << std::endl;

    std::vector<u8> buffer(file_size);
    if (file.read((char*)buffer.data(), file_size)) {
        std::cout << "[FONT] File read successfully. Forwarding to init()..." << std::endl;
        init(buffer.data(), buffer.size(), size);
        std::cout << "[FONT] init() completed." << std::endl;
    } else {
        std::cerr << "[FONT] Failed to read file data!" << std::endl;
    }
}


Font::Font(const u8* data, usize size_bytes, f32 size) {
    init(data, size_bytes, size);
}

Font::~Font() {
}

void Font::init(const u8* data, usize size_bytes, f32 size) {
    std::cout << "[FONT] init() starting. Size parameter: " << size << std::endl;
    (void)size_bytes;
    m_font_size = size;

    // We'll use a fixed atlas size for now. 512x512 is usually enough for ASCII.
    const i32 atlas_w = 512;
    const i32 atlas_h = 512;
    std::vector<u8> bitmap(atlas_w * atlas_h, 0);

    // Initialize with a dummy 1x1 empty texture to avoid null dereferences
    u8 empty_pixel[4] = {0,0,0,0};
    std::cout << "[FONT] Creating 1x1 texture..." << std::endl;
    m_atlas = std::make_unique<Texture2D>(empty_pixel, 1, 1, 4, Texture2D::FilterMode::LINEAR);

    stbtt_pack_context pc;
    std::cout << "[FONT] stbtt_PackBegin..." << std::endl;
    if (!stbtt_PackBegin(&pc, bitmap.data(), atlas_w, atlas_h, 0, 1, nullptr)) {
        std::cerr << "Failed to initialize font pack context" << std::endl;
        return;
    }

    // We only pack ASCII for now
    stbtt_packedchar packed_chars[96];
    stbtt_pack_range range{};
    range.font_size = size;
    range.first_unicode_codepoint_in_range = 32;
    range.num_chars = 96;
    range.chardata_for_range = packed_chars;

    std::cout << "[FONT] stbtt_PackFontRanges..." << std::endl;
    if (!stbtt_PackFontRanges(&pc, data, 0, &range, 1)) {
        std::cerr << "Failed to pack font ranges" << std::endl;
        stbtt_PackEnd(&pc);
        return; // m_atlas remains the dummy 1x1 texture
    }

    std::cout << "[FONT] stbtt_PackEnd..." << std::endl;
    stbtt_PackEnd(&pc);

    // Create the texture atlas. 
    // We expand the 1-channel bitmap to 4-channel RGBA (255, 255, 255, mask)
    // so that it works perfectly with the default "FragColor = texColor * v_Color" shader.
    std::vector<u8> rgba_bitmap(atlas_w * atlas_h * 4);
    for (i32 y = 0; y < atlas_h; ++y) {
        for (i32 x = 0; x < atlas_w; ++x) {
            i32 src_idx = y * atlas_w + x;
            i32 dst_idx = (atlas_h - 1 - y) * atlas_w + x;
            rgba_bitmap[dst_idx * 4 + 0] = 255;
            rgba_bitmap[dst_idx * 4 + 1] = 255;
            rgba_bitmap[dst_idx * 4 + 2] = 255;
            rgba_bitmap[dst_idx * 4 + 3] = bitmap[src_idx];
        }
    }

    m_atlas = std::make_unique<Texture2D>(rgba_bitmap.data(), atlas_w, atlas_h, 4, Texture2D::FilterMode::LINEAR);

    // Cache glyph metrics
    for (i32 i = 0; i < 96; ++i) {
        char c = (char)(32 + i);
        const auto& glyph_data = packed_chars[i];

        Glyph& g = m_glyphs[c];
        g.size = { (f32)(glyph_data.x1 - glyph_data.x0), (f32)(glyph_data.y1 - glyph_data.y0) };
        g.bearing = { (f32)glyph_data.xoff, (f32)glyph_data.yoff };
        // Restore standard Y-down UV coordinates, texture is flipped appropriately in memory.
        g.uv_min = { (f32)glyph_data.x0 / atlas_w, (f32)glyph_data.y0 / atlas_h };
        g.uv_max = { (f32)glyph_data.x1 / atlas_w, (f32)glyph_data.y1 / atlas_h };
        g.advance = glyph_data.xadvance;
    }
}

const Font::Glyph* Font::get_glyph(char c) const {
    auto it = m_glyphs.find(c);
    if (it != m_glyphs.end()) return &it->second;
    return nullptr;
}

f32 Font::measure_text(std::string_view text) const {
    f32 width = 0.0f;
    for (char c : text) {
        if (const auto* g = get_glyph(c)) {
            width += g->advance;
        }
    }
    return width;
}

} // namespace nova
