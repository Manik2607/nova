#include "nova/core.hpp"
#include "nova/platform/window.hpp"
#include "nova/input/input.hpp"
#include "nova/renderer/renderer2d.hpp"
#include "nova/renderer/text_renderer.hpp"
#include "nova/math/vector4.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "nova/ui/control.hpp"
#include "nova/ui/label.hpp"
#include "nova/ui/button.hpp"
#include "nova/ui/text_input.hpp"
#include "nova/ui/panel.hpp"
#include "nova/ui/box_container.hpp"
#include "nova/ui/grid_container.hpp"
#include "nova/ui/progress_bar.hpp"
#include "nova/ui/check_box.hpp"
#include "nova/ui/slider.hpp"
#include "nova/ui/dropdown.hpp"
#include "nova/ui/tab_bar.hpp"
#include "nova/ui/scroll_view.hpp"
#include "nova/ui/text_area.hpp"

using namespace nova;
using namespace nova::ui;

// --- Premium Color Palette ---
const Color COLOR_BG_DARK    = {0.05f, 0.05f, 0.07f, 1.0f};
const Color COLOR_SIDEBAR    = {0.08f, 0.08f, 0.12f, 1.0f};
const Color COLOR_ACCENT     = {0.4f, 0.7f, 1.0f, 1.0f};
const Color COLOR_SUCCESS    = {0.3f, 0.9f, 0.6f, 1.0f};
const Color COLOR_PANEL      = {0.12f, 0.12f, 0.18f, 1.0f};

// --- Helper for creating a "Card" container ---
Panel* create_card(Control* parent, std::string_view title, TextRenderer* tr) {
    auto card = parent->add_child<Panel>();
    card->background_color = COLOR_PANEL;
    card->padding = {25, 50, 25, 25}; // Increased padding (Left, Top, Right, Bottom)
    card->border_color = {0.2f, 0.2f, 0.3f, 1.0f};
    
    auto lbl = card->add_child<Label>(tr, title);
    lbl->position = {25, 15}; // Adjusted label position
    lbl->font_size = 15.0f;
    lbl->color = COLOR_ACCENT;
    
    return card;
}

int main() {
    Window window(1280, 720, "NovaGFX UI Sandbox");
    Renderer2D renderer;
    TextRenderer text_renderer;

    auto root_ui = std::make_unique<Control>();
    root_ui->size = {1280.0f, 720.0f};

    // --- Background & Core Layout ---
    auto bg = root_ui->add_child<Panel>();
    bg->anchor = Anchor::FILL; // Background fills the screen
    bg->background_color = COLOR_BG_DARK;

    // Use a HBox for the primary root layout (Sidebar | Workspace)
    auto root_hbox = bg->add_child<HBoxContainer>();
    root_hbox->anchor = Anchor::FILL;
    root_hbox->spacing = 0;

    // Sidebar
    auto sidebar_panel = root_hbox->add_child<Panel>();
    sidebar_panel->size = {240, 0}; // Width 240, height will be matched by HBox
    sidebar_panel->expand = true;   // Stretch height in HBox
    sidebar_panel->background_color = COLOR_SIDEBAR;
    sidebar_panel->border_thickness = 0;

    auto side_scroll = sidebar_panel->add_child<ScrollView>();
    side_scroll->anchor = Anchor::FILL;
    side_scroll->background_color = {0,0,0,0}; // Transparent to show sidebar panel bg
    
    auto side_vbox = side_scroll->content->add_child<VBoxContainer>();
    side_vbox->padding = {15, 20, 15, 20};
    side_vbox->spacing = 8.0f;
    side_vbox->expand = true;

    side_vbox->add_child<Label>(&text_renderer, "NAVIGATION")->color = COLOR_ACCENT;
    side_vbox->add_child<Panel>()->size = {180, 1}; // Divider

    // Generate 60 Navigation Buttons
    for (int i = 1; i <= 60; ++i) {
        std::string label = "Module #" + std::to_string(i);
        auto b = side_vbox->add_child<Button>(&text_renderer, label);
        b->size = {190, 35};
        if (i == 1) b->color_normal = COLOR_PANEL; // Highlight first
    }

    // Workspace (fills remaining space in HBox)
    auto workspace = root_hbox->add_child<TabBar>(&text_renderer);
    workspace->expand = true; 
    workspace->margins = {30, 30, 30, 30}; // Increased workspace margins (clearer windowing)

    // -------------------------------------------------------------------------
    // PAGE 1: COMPONENTS
    // -------------------------------------------------------------------------
    auto page_components = workspace->add_child<Panel>();
    page_components->background_color = {0,0,0,0}; // Transparent
    workspace->add_tab("Components", page_components);

    auto components_grid = page_components->add_child<GridContainer>();
    components_grid->columns = 2;
    components_grid->spacing = 30.0f; // Increased grid spacing
    components_grid->padding = {20, 20, 20, 20}; // Increased grid padding
    components_grid->anchor = Anchor::FILL; 

    // Buttons Card
    auto card_btns = create_card(components_grid, "BUTTON VARIATIONS", &text_renderer);
    card_btns->expand = true; 
    auto btn_vbox = card_btns->add_child<VBoxContainer>();
    btn_vbox->expand = true; 
    btn_vbox->spacing = 12.0f; // More room between buttons
    btn_vbox->add_child<Button>(&text_renderer, "PRIMARY ACTION")->color_normal = COLOR_ACCENT;
    btn_vbox->add_child<Button>(&text_renderer, "SUCCESS ACTION")->color_normal = COLOR_SUCCESS;
    auto danger = btn_vbox->add_child<Button>(&text_renderer, "DANGER ZONE");
    danger->color_normal = {0.8f, 0.2f, 0.2f, 1.0f};

    // Controls Card
    auto card_controls = create_card(components_grid, "INPUT CONTROLS", &text_renderer);
    card_controls->expand = true; 
    auto ctrl_vbox = card_controls->add_child<VBoxContainer>();
    ctrl_vbox->expand = true;
    ctrl_vbox->spacing = 25.0f; // More room between inputs
    ctrl_vbox->add_child<CheckBox>(&text_renderer, "Enable System Logs");
    ctrl_vbox->add_child<CheckBox>(&text_renderer, "Hardware Acceleration")->checked = true;
    
    auto dd = ctrl_vbox->add_child<Dropdown>(&text_renderer);
    dd->add_option("Display Mode: Windowed");
    dd->add_option("Display Mode: Fullscreen");
    dd->add_option("Display Mode: Borderless");

    // -------------------------------------------------------------------------
    // PAGE 2: LAYOUTS
    // -------------------------------------------------------------------------
    auto page_layouts = workspace->add_child<Panel>();
    page_layouts->background_color = {0,0,0,0};
    workspace->add_tab("Layouts", page_layouts);

    auto layout_vbox = page_layouts->add_child<VBoxContainer>();
    layout_vbox->padding = {20, 20, 20, 20};
    layout_vbox->spacing = 30.0f;

    auto row_h = layout_vbox->add_child<HBoxContainer>();
    row_h->spacing = 15.0f;
    for(int i=0; i<4; ++i) row_h->add_child<Panel>()->size = {100, 100};

    auto row_grid = layout_vbox->add_child<GridContainer>();
    row_grid->columns = 4;
    row_grid->spacing = 10.0f;
    for(int i=0; i<8; ++i) {
        auto p = row_grid->add_child<Panel>();
        p->size = {150, 60};
        p->background_color = {0.2f, 0.2f, 0.25f, 1.0f};
    }

    // -------------------------------------------------------------------------
    // PAGE 3: INTERACTIVE (Linking controls)
    // -------------------------------------------------------------------------
    auto page_interactive = workspace->add_child<Panel>();
    page_interactive->background_color = {0,0,0,0};
    workspace->add_tab("Interactions", page_interactive);

    auto int_card = create_card(page_interactive, "DATA LINKING DEMO", &text_renderer);
    int_card->size = {500, 300};
    int_card->position = {50, 50};
    
    auto int_vbox = int_card->add_child<VBoxContainer>();
    int_vbox->spacing = 25.0f;
    
    auto prog_label = int_vbox->add_child<Label>(&text_renderer, "System Load: 50%");
    auto progress = int_vbox->add_child<ProgressBar>();
    progress->value = 0.5f;
    progress->size.x = 400;

    auto slider = int_vbox->add_child<Slider>();
    slider->size.x = 400;
    slider->on_changed = [progress, prog_label](f32 v) {
        progress->value = v;
        prog_label->text = "System Load: " + std::to_string((int)(v * 100)) + "%";
        if (v > 0.8f) progress->color_fill = {1.0f, 0.3f, 0.3f, 1.0f};
        else progress->color_fill = COLOR_SUCCESS;
    };

    // -------------------------------------------------------------------------
    // PAGE 4: SCROLLING
    // -------------------------------------------------------------------------
    auto page_scroll = workspace->add_child<ScrollView>();
    workspace->add_tab("Scrolling", page_scroll);
    
    auto scroll_vbox = page_scroll->content->add_child<VBoxContainer>();
    scroll_vbox->padding = {40, 40, 40, 40};
    scroll_vbox->spacing = 15.0f;
    for (int i = 0; i < 50; ++i) {
        auto item = scroll_vbox->add_child<Panel>();
        item->size = {780, 50}; // Slightly taller items
        item->background_color = {0.15f, 0.15f, 0.2f, 1.0f};
        auto item_label = item->add_child<Label>(&text_renderer, "Registry Entry #" + std::to_string(i));
        item_label->position = {25, 12};
    }

    // --- HUD & Overlay ---
    auto hud = root_ui->add_child<Panel>();
    hud->background_color = {0,0,0,0.5f};
    hud->size = {220, 60};
    hud->anchor = Anchor::TOP_RIGHT;
    hud->margins = {0, 20, 20, 0};
    
    auto fps_lbl = hud->add_child<Label>(&text_renderer, "FPS: 000");
    fps_lbl->anchor = Anchor::CENTER;
    fps_lbl->color = COLOR_SUCCESS;

    f32 last_time = 0.0f;
    f32 fps_timer = 0.0f;
    int frames = 0;

    while (!window.should_close()) {
        f32 current_time = static_cast<f32>(glfwGetTime());
        f32 delta = current_time - last_time;
        last_time = current_time;

        fps_timer += delta;
        frames++;
        if (fps_timer >= 1.0f) {
            fps_lbl->text = "FPS: " + std::to_string(frames);
            fps_timer = 0.0f;
            frames = 0;
        }

        Input::update();
        window.poll_events();

        if (Input::is_key_pressed(Key::ESCAPE)) window.close();
        
        // Handle window resizing
        Vector2i win_size = window.get_size();
        root_ui->size = {(f32)win_size.x, (f32)win_size.y};

        root_ui->update_ui_tree(delta, Input::get_mouse_position(), 
                               Input::is_mouse_pressed(MouseButton::LEFT), 
                               Input::is_mouse_released(MouseButton::LEFT));

        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.begin();
        renderer.init_camera({win_size.x / 2.0f, win_size.y / 2.0f}, 1.0f, 
                            {(f32)win_size.x, (f32)win_size.y});
        
        root_ui->draw_tree(renderer);
        renderer.end();

        window.swap_buffers();
    }

    return 0;
}
