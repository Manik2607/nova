#pragma once
#include "nova/ui/control.hpp"
#include "nova/ui/button.hpp"
#include "nova/ui/box_container.hpp"

namespace nova::ui {

class Dropdown : public Control {
public:
    std::string selected_option;
    std::vector<std::string> options;
    std::function<void(std::string)> on_selection_changed;

    TextRenderer* text_renderer{nullptr};

    Dropdown(TextRenderer* tr) : text_renderer(tr) {
        size = {150, 30};
        
        m_main_button = add_child<Button>(tr, "Select...");
        m_main_button->size = size;
        m_main_button->on_pressed = [this]() {
            m_expanded = !m_expanded;
            m_list_container->set_visible(m_expanded);
        };

        m_list_container = add_child<VBoxContainer>();
        m_list_container->position = {0, size.y};
        m_list_container->set_visible(false);
        // We'll populate buttons later
    }

    void add_option(std::string_view opt) {
        options.push_back(std::string(opt));
        auto btn = m_list_container->add_child<Button>(text_renderer, opt);
        btn->size = {size.x, size.y};
        btn->on_pressed = [this, s = std::string(opt)]() {
            selected_option = s;
            m_main_button->text = s;
            m_expanded = false;
            m_list_container->set_visible(false);
            if (on_selection_changed) on_selection_changed(s);
        };
    }

private:
    Button* m_main_button{nullptr};
    VBoxContainer* m_list_container{nullptr};
    bool m_expanded{false};
};

} // namespace nova::ui
