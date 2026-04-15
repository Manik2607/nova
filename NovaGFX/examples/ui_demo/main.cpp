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

// ─────────────────────────────────────────────────────────────────────────────
// Colour palette
// ─────────────────────────────────────────────────────────────────────────────
const Color BG_DARK    = {0.05f, 0.05f, 0.07f, 1.0f};
const Color SIDEBAR    = {0.08f, 0.08f, 0.12f, 1.0f};
const Color ACCENT     = {0.45f, 0.72f, 1.0f,  1.0f};
const Color SUCCESS    = {0.30f, 0.90f, 0.60f, 1.0f};
const Color DANGER     = {0.85f, 0.25f, 0.25f, 1.0f};
const Color CARD_BG    = {0.11f, 0.11f, 0.17f, 1.0f};
const Color DIVIDER    = {0.22f, 0.22f, 0.32f, 1.0f};
const Color TEXT_DIM   = {0.55f, 0.58f, 0.65f, 1.0f};
const Color TAB_ACTIVE = {0.22f, 0.32f, 0.52f, 1.0f};
const Color TAB_IDLE   = {0.10f, 0.10f, 0.15f, 1.0f};

// ─────────────────────────────────────────────────────────────────────────────
// create_card()
// Creates a styled Panel card with a section-title label + returns
// a VBoxContainer (FILL, expanding) that sits below the title so children
// don't overlap the header.
// ─────────────────────────────────────────────────────────────────────────────
VBoxContainer* create_card(Control* parent, std::string_view title,
                           TextRenderer* tr, f32 card_height = 300.0f,
                           f32 spacing = 12.0f)
{
    auto card = parent->add_child<Panel>();
    card->background_color  = CARD_BG;
    card->border_color      = DIVIDER;
    card->border_thickness  = 1.0f;
    card->size              = {200.0f, card_height}; // width overridden by grid

    // Section title label — lives at the very top
    auto lbl = card->add_child<Label>(tr, title);
    lbl->position  = {18, 14};
    lbl->font_size = 12.0f;
    lbl->color     = ACCENT;

    // Thin separator below title
    auto sep = card->add_child<Panel>();
    sep->position           = {16, 36};
    sep->size               = {168, 1};
    sep->background_color   = DIVIDER;
    sep->border_thickness   = 0;

    // Content VBox anchored to fill card, margins push it below the header
    auto vbox = card->add_child<VBoxContainer>();
    vbox->anchor   = Anchor::FILL;
    vbox->margins  = {16, 44, 16, 16}; // left, top, right, bottom
    vbox->expand   = true;             // don't auto-size; FILL handles it
    vbox->spacing  = spacing;

    return vbox;
}

int main() {
    Window     window(1280, 720, "NovaGFX UI Sandbox");
    Renderer2D renderer;
    TextRenderer text_renderer;

    auto root_ui    = std::make_unique<Control>();
    root_ui->size   = {1280.0f, 720.0f};

    // ─── Full-screen background ───────────────────────────────────────────
    auto bg              = root_ui->add_child<Panel>();
    bg->anchor           = Anchor::FILL;
    bg->background_color = BG_DARK;
    bg->border_thickness = 0;

    // ─── Root layout: sidebar | workspace ────────────────────────────────
    auto root_hbox  = bg->add_child<HBoxContainer>();
    root_hbox->anchor  = Anchor::FILL;
    root_hbox->spacing = 0;

    // ═════════════════════════════════════════════════════════════════════
    // SIDEBAR  (220 px wide, full height)
    // ═════════════════════════════════════════════════════════════════════
    auto sidebar        = root_hbox->add_child<Panel>();
    sidebar->size       = {220, 0};
    sidebar->expand     = true;        // match HBox height
    sidebar->background_color = SIDEBAR;
    sidebar->border_thickness = 0;

    auto side_scroll               = sidebar->add_child<ScrollView>();
    side_scroll->anchor            = Anchor::FILL;
    side_scroll->background_color  = {0, 0, 0, 0};
    side_scroll->scroll_speed      = 55.0f;

    auto side_vbox    = side_scroll->content->add_child<VBoxContainer>();
    side_vbox->padding = {12, 16, 20, 16};  // right=20 → room for scroll bar
    side_vbox->spacing = 6.0f;

    // "NAVIGATION" heading
    {
        auto h = side_vbox->add_child<Label>(&text_renderer, "NAVIGATION");
        h->font_size = 11.0f;
        h->color     = ACCENT;
    }
    // Divider
    {
        auto d = side_vbox->add_child<Panel>();
        d->size             = {180, 1};
        d->background_color = DIVIDER;
        d->border_thickness = 0;
    }

    // 60 nav buttons
    for (int i = 1; i <= 60; ++i) {
        auto b = side_vbox->add_child<Button>(&text_renderer, "Module #" + std::to_string(i));
        b->size         = {188, 34};
        b->font_size    = 14.0f;
        b->color_normal = (i == 1) ? TAB_ACTIVE : Color{0.12f, 0.12f, 0.18f, 1.0f};
        b->color_hover  = {0.17f, 0.20f, 0.30f, 1.0f};
    }

    // ═════════════════════════════════════════════════════════════════════
    // WORKSPACE  (TabBar, fills remaining width)
    // ═════════════════════════════════════════════════════════════════════
    auto workspace  = root_hbox->add_child<TabBar>(&text_renderer);
    workspace->expand = true;

    // ─────────────────────────────────────────────────────────────────
    // PAGE 1 — COMPONENTS
    // ─────────────────────────────────────────────────────────────────
    auto page_comp  = workspace->add_child<Panel>();
    page_comp->background_color  = {0, 0, 0, 0};
    page_comp->border_thickness  = 0;
    workspace->add_tab("Components", page_comp);

    // Grid fills the page, two equal columns
    auto grid = page_comp->add_child<GridContainer>();
    grid->anchor  = Anchor::FILL;
    grid->expand  = true;
    grid->columns = 2;
    grid->spacing = 18.0f;
    grid->padding = {16, 16, 16, 16};

    // ── Card 1: Button Variations ──
    {
        auto vb = create_card(grid, "BUTTON VARIATIONS", &text_renderer, 300.0f, 10.0f);
        {
            auto b = vb->add_child<Button>(&text_renderer, "PRIMARY ACTION");
            b->expand       = true;
            b->font_size    = 16.0f;
            b->color_normal = ACCENT;
            b->color_text   = {0.04f, 0.04f, 0.06f, 1.0f};
        }
        {
            auto b = vb->add_child<Button>(&text_renderer, "SUCCESS ACTION");
            b->expand       = true;
            b->font_size    = 16.0f;
            b->color_normal = SUCCESS;
            b->color_text   = {0.04f, 0.04f, 0.06f, 1.0f};
        }
        {
            auto b = vb->add_child<Button>(&text_renderer, "DANGER ZONE");
            b->expand       = true;
            b->font_size    = 16.0f;
            b->color_normal = DANGER;
        }
        {
            auto b = vb->add_child<Button>(&text_renderer, "SECONDARY");
            b->expand       = true;
            b->font_size    = 16.0f;
            b->color_normal = {0.14f, 0.14f, 0.22f, 1.0f};
        }
    }

    // ── Card 2: Input Controls ──
    {
        auto vb = create_card(grid, "INPUT CONTROLS", &text_renderer, 300.0f, 18.0f);
        {
            auto c = vb->add_child<CheckBox>(&text_renderer, "Enable System Logs");
            c->size      = {200, 24};
            c->font_size = 15.0f;
        }
        {
            auto c = vb->add_child<CheckBox>(&text_renderer, "Hardware Acceleration");
            c->size      = {200, 24};
            c->checked   = true;
            c->font_size = 15.0f;
        }
        {
            auto lbl = vb->add_child<Label>(&text_renderer, "Display Mode");
            lbl->font_size = 14.0f;
            lbl->color     = TEXT_DIM;
        }
        {
            auto dd = vb->add_child<Dropdown>(&text_renderer);
            dd->size = {180, 34};
            dd->add_option("Windowed");
            dd->add_option("Fullscreen");
            dd->add_option("Borderless Windowed");
        }
    }

    // ─────────────────────────────────────────────────────────────────
    // PAGE 2 — LAYOUTS
    // ─────────────────────────────────────────────────────────────────
    auto page_lay   = workspace->add_child<Panel>();
    page_lay->background_color = {0, 0, 0, 0};
    page_lay->border_thickness = 0;
    workspace->add_tab("Layouts", page_lay);

    // VBox that fills the page
    auto lay_vbox   = page_lay->add_child<VBoxContainer>();
    lay_vbox->anchor  = Anchor::FILL;
    lay_vbox->expand  = true;
    lay_vbox->padding = {20, 20, 20, 20};
    lay_vbox->spacing = 18.0f;

    // ── HBoxContainer demo ──
    {
        auto h = lay_vbox->add_child<Label>(&text_renderer, "HBoxContainer");
        h->font_size = 14.0f;
        h->color     = ACCENT;
    }
    {
        auto row = lay_vbox->add_child<HBoxContainer>();
        row->spacing = 10.0f;
        const Color cols[] = {
            {0.28f, 0.42f, 0.70f, 1.0f}, {0.54f, 0.30f, 0.65f, 1.0f},
            {0.28f, 0.65f, 0.42f, 1.0f}, {0.70f, 0.42f, 0.28f, 1.0f},
        };
        for (int i = 0; i < 4; ++i) {
            auto p = row->add_child<Panel>();
            p->size             = {130, 90};
            p->background_color = cols[i];
            p->border_thickness = 0;
            auto lbl = p->add_child<Label>(&text_renderer, "Box " + std::to_string(i + 1));
            lbl->position  = {12, 10};
            lbl->font_size = 15.0f;
        }
    }

    // ── GridContainer demo ──
    {
        auto h = lay_vbox->add_child<Label>(&text_renderer, "GridContainer  (4 columns)");
        h->font_size = 14.0f;
        h->color     = ACCENT;
    }
    {
        auto g = lay_vbox->add_child<GridContainer>();
        g->columns = 4;
        g->spacing = 8.0f;
        for (int i = 0; i < 8; ++i) {
            auto p = g->add_child<Panel>();
            p->size             = {0, 60};   // width=0 → adaptive
            p->background_color = Color{0.16f + i * 0.015f, 0.14f, 0.24f, 1.0f};
            p->border_thickness = 0;
            auto lbl = p->add_child<Label>(&text_renderer, "Cell " + std::to_string(i + 1));
            lbl->position  = {10, 10};
            lbl->font_size = 14.0f;
        }
    }

    // ─────────────────────────────────────────────────────────────────
    // PAGE 3 — INTERACTIONS
    // ─────────────────────────────────────────────────────────────────
    auto page_int   = workspace->add_child<Panel>();
    page_int->background_color = {0, 0, 0, 0};
    page_int->border_thickness = 0;
    workspace->add_tab("Interactions", page_int);

    // Manually placed card (not inside a grid, so we control its size explicitly)
    auto card_panel = page_int->add_child<Panel>();
    card_panel->size            = {520, 300};
    card_panel->position        = {40, 40};
    card_panel->background_color = CARD_BG;
    card_panel->border_color    = DIVIDER;
    card_panel->border_thickness = 1.0f;

    {
        auto lbl = card_panel->add_child<Label>(&text_renderer, "DATA LINKING DEMO");
        lbl->position  = {18, 14};
        lbl->font_size = 12.0f;
        lbl->color     = ACCENT;
    }
    {
        auto sep = card_panel->add_child<Panel>();
        sep->position           = {16, 36};
        sep->size               = {488, 1};
        sep->background_color   = DIVIDER;
        sep->border_thickness   = 0;
    }

    // Content VBox below header
    auto int_vbox  = card_panel->add_child<VBoxContainer>();
    int_vbox->anchor  = Anchor::FILL;
    int_vbox->margins = {20, 50, 20, 20};
    int_vbox->expand  = true;
    int_vbox->spacing = 14.0f;

    Label*       prog_lbl = nullptr;
    ProgressBar* prog_bar = nullptr;

    {
        prog_lbl = int_vbox->add_child<Label>(&text_renderer, "System Load:  50 %");
        prog_lbl->font_size = 16.0f;
    }
    {
        prog_bar = int_vbox->add_child<ProgressBar>();
        prog_bar->value  = 0.5f;
        prog_bar->size   = {0, 18};
        prog_bar->expand = false;
    }
    {
        auto lbl = int_vbox->add_child<Label>(&text_renderer, "Drag the slider:");
        lbl->font_size = 14.0f;
        lbl->color     = TEXT_DIM;
    }
    {
        auto slider = int_vbox->add_child<Slider>();
        slider->size  = {0, 24};
        slider->value = 0.5f;
        slider->on_changed = [prog_bar, prog_lbl](f32 v) {
            prog_bar->value = v;
            prog_lbl->text  = "System Load:  " + std::to_string((int)(v * 100)) + " %";
            if      (v > 0.80f) prog_bar->color_fill = DANGER;
            else if (v > 0.55f) prog_bar->color_fill = {1.0f, 0.75f, 0.20f, 1.0f};
            else                prog_bar->color_fill = SUCCESS;
        };
    }

    // ─────────────────────────────────────────────────────────────────
    // PAGE 4 — SCROLLING
    // ─────────────────────────────────────────────────────────────────
    auto page_scroll = workspace->add_child<ScrollView>();
    page_scroll->background_color = {0, 0, 0, 0};
    page_scroll->scroll_speed     = 55.0f;
    workspace->add_tab("Scrolling", page_scroll);

    auto sv_vbox            = page_scroll->content->add_child<VBoxContainer>();
    sv_vbox->padding         = {20, 20, 28, 20};  // right=28 → clear of scrollbar
    sv_vbox->spacing         = 8.0f;

    {
        auto h = sv_vbox->add_child<Label>(&text_renderer,
                 "Registry Entries  (scroll with mouse wheel or drag the bar)");
        h->font_size = 15.0f;
        h->color     = ACCENT;
    }

    for (int i = 0; i < 50; ++i) {
        auto item = sv_vbox->add_child<Panel>();
        item->size             = {0, 46};  // width adaptive
        item->background_color = Color{0.12f + (i % 3) * 0.015f, 0.12f, 0.19f, 1.0f};
        item->border_color     = DIVIDER;
        item->border_thickness = 1.0f;

        auto lbl = item->add_child<Label>(&text_renderer, "Registry Entry #" + std::to_string(i));
        lbl->position  = {18, 13};
        lbl->font_size = 15.0f;
    }

    // ═════════════════════════════════════════════════════════════════════
    // HUD — FPS counter (top-right overlay)
    // ═════════════════════════════════════════════════════════════════════
    auto hud                = root_ui->add_child<Panel>();
    hud->background_color   = {0.05f, 0.05f, 0.09f, 0.88f};
    hud->size               = {130, 36};
    hud->anchor             = Anchor::TOP_RIGHT;
    hud->margins            = {0, 10, 10, 0};
    hud->border_color       = DIVIDER;
    hud->border_thickness   = 1.0f;

    // Manual position so the text baseline sits visually centred in the box.
    // (Anchor::CENTER on Label needs update_size() to work; manual is more
    //  predictable until the layout loop runs once.)
    auto fps_lbl = hud->add_child<Label>(&text_renderer, "FPS: --");
    fps_lbl->font_size = 15.0f;
    fps_lbl->color     = SUCCESS;
    fps_lbl->anchor    = Anchor::CENTER;   // will work after layout_children runs update_size

    // ═════════════════════════════════════════════════════════════════════
    // Main loop
    // ═════════════════════════════════════════════════════════════════════
    f32 last_time = 0.0f;
    f32 fps_timer = 0.0f;
    int frames    = 0;

    while (!window.should_close()) {
        f32 current_time = static_cast<f32>(glfwGetTime());
        f32 delta        = current_time - last_time;
        last_time        = current_time;

        fps_timer += delta;
        frames++;
        if (fps_timer >= 1.0f) {
            fps_lbl->text = "FPS: " + std::to_string(frames);
            fps_timer     = 0.0f;
            frames        = 0;
        }

        Input::update();
        window.poll_events();

        if (Input::is_key_pressed(Key::ESCAPE)) window.close();

        Vector2i win_size = window.get_size();
        root_ui->size     = {(f32)win_size.x, (f32)win_size.y};

        root_ui->update_ui_tree(delta,
                                Input::get_mouse_position(),
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
