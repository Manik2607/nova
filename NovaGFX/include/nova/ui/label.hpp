#pragma once
#include "nova/ui/control.hpp"
#include "nova/renderer/text_renderer.hpp"
#include <string>

namespace nova::ui {

class Label : public Control {
public:
    std::string text;
    f32 font_size{24.0f};
    Color color{Color::WHITE()};
    TextRenderer* text_renderer{nullptr};

    Label(TextRenderer* tr, std::string_view initial_text = "")
        : text_renderer(tr), text(initial_text) {}

    void draw(Renderer2D& renderer) override;

    // Optional: auto-adjust size based on text
    void update_size();
};

} // namespace nova::ui
