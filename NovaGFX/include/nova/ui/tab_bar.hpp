#pragma once
#include "nova/ui/control.hpp"
#include "nova/ui/button.hpp"
#include "nova/ui/box_container.hpp"

namespace nova::ui {

class TabBar : public Control {
public:
    struct Tab {
        std::string name;
        Control* panel;
        Button* button;
    };

    TextRenderer* text_renderer{nullptr};

    TabBar(TextRenderer* tr) : text_renderer(tr) {
        size = {400, 300};
        
        m_button_container = add_child<HBoxContainer>();
        m_button_container->spacing = 2.0f;
    }

    void add_tab(std::string_view name, Control* panel) {
        auto btn = m_button_container->add_child<Button>(text_renderer, name);
        btn->size = {80, 30};
        
        Tab tab{std::string(name), panel, btn};
        
        btn->on_pressed = [this, index = m_tabs.size()]() {
            set_tab(index);
        };
        
        m_tabs.push_back(tab);
        
        if (m_tabs.size() == 1) {
            set_tab(0);
        } else {
            panel->set_visible(false);
        }
    }

    void set_tab(usize index) {
        if (index >= m_tabs.size()) return;
        
        for (usize i = 0; i < m_tabs.size(); ++i) {
            bool active = (i == index);
            m_tabs[i].panel->set_visible(active);
            m_tabs[i].button->color_normal = active ? Color(0.3f, 0.3f, 0.4f, 1.0f) : Color(0.15f, 0.15f, 0.15f, 1.0f);
        }
    }

private:
    HBoxContainer* m_button_container{nullptr};
    std::vector<Tab> m_tabs;
};

} // namespace nova::ui
