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
    f32 tab_height{32.0f};     // Height of the tab button strip
    Color color_active_tab{0.25f, 0.35f, 0.55f, 1.0f};
    Color color_inactive_tab{0.12f, 0.12f, 0.18f, 1.0f};
    Color color_strip_bg{0.08f, 0.08f, 0.12f, 1.0f};

    TabBar(TextRenderer* tr) : text_renderer(tr) {
        size = {400, 300};
        m_button_container = add_child<HBoxContainer>();
        m_button_container->spacing = 2.0f;
    }

    void add_tab(std::string_view name, Control* panel) {
        // Add the panel as our direct child
        // (it should already be a child if added via add_child before calling add_tab)
        // If not a child of ours, adopt it:
        bool already_child = false;
        for (auto& c : m_children) {
            if (c.get() == panel) { already_child = true; break; }
        }
        // Don't re-add — panel ownership stays with caller (add_child already transferred it)

        // Create tab button
        auto btn = m_button_container->add_child<Button>(text_renderer, name);
        btn->size = {100, tab_height};
        btn->color_normal = color_inactive_tab;
        btn->color_hover  = Color{0.18f, 0.22f, 0.32f, 1.0f};
        btn->font_size = 14.0f;

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
            m_tabs[i].button->color_normal = active ? color_active_tab : color_inactive_tab;
        }
        m_active_tab = index;
    }

    void layout_children() override {
        // Button strip at the top
        m_button_container->position = {0, 0};
        m_button_container->size = {size.x, tab_height};

        // Make each tab button proportional
        if (!m_tabs.empty()) {
            f32 btn_w = (size.x - m_button_container->spacing * (m_tabs.size() - 1)) / m_tabs.size();
            for (auto& tab : m_tabs) {
                tab.button->size = {btn_w, tab_height};
            }
        }
        m_button_container->layout_children();

        // All panels go below the strip and fill remaining space
        Vector2f panel_pos  = {0, tab_height + 1};
        Vector2f panel_size = {size.x, size.y - tab_height - 1};

        for (auto& tab : m_tabs) {
            // Only layout panels that are our direct children.
            // If they are children of something else (like a ScrollView), 
            // their own parent will handle their layout.
            bool is_direct_child = false;
            for (auto& c : m_children) {
                if (c.get() == tab.panel) { is_direct_child = true; break; }
            }

            if (is_direct_child) {
                tab.panel->position = panel_pos;
                tab.panel->size     = panel_size;
            }

            if (tab.panel->visible) {
                tab.panel->layout_children();
            }
        }
    }

    void draw(Renderer2D& renderer) override {
        // Draw tab strip background
        Rect2f rect = get_global_rect();
        renderer.draw_rect(rect.position,
                           {rect.size.x, tab_height},
                           color_strip_bg);
        // Separator line between strip and content
        renderer.draw_rect({rect.position.x, rect.position.y + tab_height},
                           {rect.size.x, 1.0f},
                           Color{0.25f, 0.25f, 0.4f, 1.0f});
    }

private:
    HBoxContainer* m_button_container{nullptr};
    std::vector<Tab> m_tabs;
    usize m_active_tab{0};
};

} // namespace nova::ui
