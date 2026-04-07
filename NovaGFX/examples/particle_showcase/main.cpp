/**
 * NovaGFX — Particle Showcase
 *
 * Demonstrates multiple particle emitters (Explosion, Fire, Smoke, Sparks,
 * Magic, Rain, Snow, Confetti) with a live-tweaking UI panel.
 *
 * Controls:
 *   1-8    – cycle to next emitter type (shown in sidebar) [Only when not typing]
 *   SPACE  – burst trigger (for burst-mode emitters) [Only when not typing]
 *   Scroll – zoom in/out
 *   LMB    – click canvas to position the active emitter
 *   ESC    – quit
 */

#include <nova/nova.hpp>
#include <nova/scene/particle_system.hpp>
#include <nova/scene/camera2d.hpp>
#include <nova/renderer/text_renderer.hpp>
#include <nova/ui/control.hpp>
#include <nova/ui/label.hpp>
#include <nova/ui/button.hpp>
#include <nova/ui/text_input.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace nova;

// ============================================================
// Constants
// ============================================================
static constexpr int   WIN_W      = 1400;
static constexpr int   WIN_H      = 800;
static constexpr float PANEL_W    = 320.0f;
static constexpr float PANEL_X    = static_cast<float>(WIN_W) - PANEL_W - 6.0f;
static constexpr float UI_FONT    = 16.0f;
static constexpr float ROW_H      = 28.0f;
static constexpr float ROW_GAP    = 4.0f;
static constexpr float LABEL_W    = 130.0f;
static constexpr float INPUT_W    = 90.0f;

// ============================================================
// Emitter preset definitions
// ============================================================
struct EmitterPreset {
    std::string name;
    ParticleEmitter emitter;
};

static std::array<EmitterPreset, 8> make_presets() {
    std::array<EmitterPreset, 8> p;

    // ── 0: EXPLOSION ──
    p[0].name = "Explosion";
    p[0].emitter.mode         = EmitterMode::BURST;
    p[0].emitter.burst_count  = 80;
    p[0].emitter.rate         = 0.0f;
    p[0].emitter.spread_angle = 3.14159f;   // full circle
    p[0].emitter.velocity_min = 200.0f;
    p[0].emitter.velocity_max = 600.0f;
    p[0].emitter.lifetime_min = 0.3f;
    p[0].emitter.lifetime_max = 0.9f;
    p[0].emitter.size_start   = 12.0f;
    p[0].emitter.size_end     = 1.0f;
    p[0].emitter.color_start  = Color(1.0f, 0.7f, 0.1f, 1.0f);
    p[0].emitter.color_end    = Color(0.7f, 0.1f, 0.0f, 0.0f);
    p[0].emitter.gravity_scale = 1.0f;
    p[0].emitter.drag         = 1.5f;
    p[0].emitter.active       = true;

    // ── 1: FIRE ──
    p[1].name = "Fire";
    p[1].emitter.mode         = EmitterMode::CONTINUOUS;
    p[1].emitter.rate         = 80.0f;
    p[1].emitter.spread_angle = 0.4f;
    p[1].emitter.rotation     = -1.5708f;   // upward (-Y)
    p[1].emitter.velocity_min = 60.0f;
    p[1].emitter.velocity_max = 180.0f;
    p[1].emitter.lifetime_min = 0.4f;
    p[1].emitter.lifetime_max = 1.1f;
    p[1].emitter.size_start   = 20.0f;
    p[1].emitter.size_end     = 4.0f;
    p[1].emitter.color_start  = Color(1.0f, 0.5f, 0.0f, 1.0f);
    p[1].emitter.color_end    = Color(0.8f, 0.1f, 0.0f, 0.0f);
    p[1].emitter.gravity_scale = -0.2f;    // float upward
    p[1].emitter.drag         = 2.0f;
    p[1].emitter.active       = true;

    // ── 2: SMOKE ──
    p[2].name = "Smoke";
    p[2].emitter.mode         = EmitterMode::CONTINUOUS;
    p[2].emitter.rate         = 30.0f;
    p[2].emitter.spread_angle = 0.5f;
    p[2].emitter.rotation     = -1.5708f;
    p[2].emitter.velocity_min = 20.0f;
    p[2].emitter.velocity_max = 60.0f;
    p[2].emitter.lifetime_min = 1.0f;
    p[2].emitter.lifetime_max = 2.5f;
    p[2].emitter.size_start   = 14.0f;
    p[2].emitter.size_end     = 55.0f;
    p[2].emitter.color_start  = Color(0.7f, 0.7f, 0.7f, 0.6f);
    p[2].emitter.color_end    = Color(0.4f, 0.4f, 0.4f, 0.0f);
    p[2].emitter.gravity_scale = -0.05f;
    p[2].emitter.drag         = 2.5f;
    p[2].emitter.active       = true;

    // ── 3: SPARKS ──
    p[3].name = "Sparks";
    p[3].emitter.mode         = EmitterMode::CONTINUOUS;
    p[3].emitter.rate         = 60.0f;
    p[3].emitter.spread_angle = 0.8f;
    p[3].emitter.rotation     = -1.5708f;
    p[3].emitter.velocity_min = 150.0f;
    p[3].emitter.velocity_max = 400.0f;
    p[3].emitter.lifetime_min = 0.2f;
    p[3].emitter.lifetime_max = 0.6f;
    p[3].emitter.size_start   = 4.0f;
    p[3].emitter.size_end     = 1.0f;
    p[3].emitter.color_start  = Color(1.0f, 1.0f, 0.5f, 1.0f);
    p[3].emitter.color_end    = Color(1.0f, 0.3f, 0.0f, 0.0f);
    p[3].emitter.gravity_scale = 1.8f;
    p[3].emitter.drag         = 0.5f;
    p[3].emitter.active       = true;

    // ── 4: MAGIC ──
    p[4].name = "Magic";
    p[4].emitter.mode         = EmitterMode::CONTINUOUS;
    p[4].emitter.rate         = 50.0f;
    p[4].emitter.spread_angle = 3.14159f;
    p[4].emitter.velocity_min = 30.0f;
    p[4].emitter.velocity_max = 120.0f;
    p[4].emitter.lifetime_min = 0.6f;
    p[4].emitter.lifetime_max = 1.8f;
    p[4].emitter.size_start   = 8.0f;
    p[4].emitter.size_end     = 0.0f;
    p[4].emitter.color_start  = Color(0.5f, 0.2f, 1.0f, 1.0f);
    p[4].emitter.color_end    = Color(0.9f, 0.4f, 1.0f, 0.0f);
    p[4].emitter.gravity_scale = -20.0f;
    p[4].emitter.drag         = 1.5f;
    p[4].emitter.active       = true;

    // ── 5: RAIN ──
    p[5].name = "Rain";
    p[5].emitter.mode         = EmitterMode::CONTINUOUS;
    p[5].emitter.rate         = 120.0f;
    p[5].emitter.spread_angle = 0.05f;
    p[5].emitter.rotation     = 1.5708f;    // downward (+Y)
    p[5].emitter.velocity_min = 400.0f;
    p[5].emitter.velocity_max = 600.0f;
    p[5].emitter.lifetime_min = 0.4f;
    p[5].emitter.lifetime_max = 0.9f;
    p[5].emitter.size_start   = 3.0f;
    p[5].emitter.size_end     = 2.0f;
    p[5].emitter.color_start  = Color(0.6f, 0.8f, 1.0f, 0.9f);
    p[5].emitter.color_end    = Color(0.6f, 0.8f, 1.0f, 0.0f);
    p[5].emitter.gravity_scale = 0.0f;
    p[5].emitter.drag         = 0.1f;
    p[5].emitter.active       = true;

    // ── 6: SNOW ──
    p[6].name = "Snow";
    p[6].emitter.mode         = EmitterMode::CONTINUOUS;
    p[6].emitter.rate         = 40.0f;
    p[6].emitter.spread_angle = 0.6f;
    p[6].emitter.rotation     = 1.5708f;
    p[6].emitter.velocity_min = 30.0f;
    p[6].emitter.velocity_max = 80.0f;
    p[6].emitter.lifetime_min = 2.0f;
    p[6].emitter.lifetime_max = 4.0f;
    p[6].emitter.size_start   = 5.0f;
    p[6].emitter.size_end     = 3.0f;
    p[6].emitter.color_start  = Color(0.9f, 0.95f, 1.0f, 1.0f);
    p[6].emitter.color_end    = Color(0.9f, 0.95f, 1.0f, 0.0f);
    p[6].emitter.gravity_scale = 0.15f;
    p[6].emitter.drag         = 3.0f;
    p[6].emitter.active       = true;

    // ── 7: CONFETTI ──
    p[7].name = "Confetti";
    p[7].emitter.mode         = EmitterMode::BURST;
    p[7].emitter.burst_count  = 120;
    p[7].emitter.rate         = 0.0f;
    p[7].emitter.spread_angle = 3.14159f;
    p[7].emitter.velocity_min = 100.0f;
    p[7].emitter.velocity_max = 350.0f;
    p[7].emitter.lifetime_min = 1.2f;
    p[7].emitter.lifetime_max = 3.0f;
    p[7].emitter.size_start   = 8.0f;
    p[7].emitter.size_end     = 5.0f;
    p[7].emitter.color_start  = Color(1.0f, 0.3f, 0.6f, 1.0f);
    p[7].emitter.color_end    = Color(0.3f, 0.8f, 1.0f, 0.0f);
    p[7].emitter.gravity_scale = 0.9f;
    p[7].emitter.drag         = 1.0f;
    p[7].emitter.active       = true;

    return p;
}

// ============================================================
// Panel — draws a semi-transparent background
// ============================================================
class Panel : public ui::Control {
public:
    Color bg_color{0.08f, 0.08f, 0.14f, 0.82f};
    Color border_color{0.3f, 0.3f, 0.5f, 1.0f};
    f32   corner_pad{0.0f};

    void draw(Renderer2D& renderer) override {
        Transform2D g = get_global_transform();
        renderer.draw_rect(g.origin, size, bg_color);
        renderer.draw_rect_outline(g.origin, size, border_color, 2.0f);
    }
};

// ============================================================
// Helper Struct for Field Mapping
// ============================================================
struct FieldMapping {
    ui::TextInput* input;
    std::function<float(const ParticleEmitter&)> getter;
};

// ============================================================
// Helper — add a float-input row to a panel with label
// ============================================================
static float s_row_y = 0.0f;

static ui::TextInput* add_float_row(ui::Control* parent, TextRenderer* tr,
    float panel_x, const std::string& name, float value, float step,
    std::function<void(float)> on_commit)
{
    auto lbl = std::make_unique<ui::Label>(tr, name);
    lbl->position  = {panel_x + 8.0f, s_row_y + 6.0f};
    lbl->font_size = UI_FONT;
    lbl->color     = {0.85f, 0.85f, 0.95f, 1.0f};
    parent->add_child(std::move(lbl));

    auto inp = std::make_unique<ui::TextInput>(tr, "", ui::InputMode::FLOAT);
    inp->set_float(value, 2);
    inp->position  = {panel_x + LABEL_W + 8.0f, s_row_y};
    inp->size      = {INPUT_W, ROW_H};
    inp->font_size = UI_FONT;
    inp->step      = step;
    inp->color_bg_normal  = {0.05f, 0.06f, 0.12f, 1.0f};
    inp->color_bg_focused = {0.12f, 0.14f, 0.28f, 1.0f};
    inp->color_border     = {0.35f, 0.35f, 0.65f, 1.0f};
    inp->on_committed = [on_commit](const std::string& s) {
        try { on_commit(std::stof(s)); } catch (...) {}
    };
    ui::TextInput* ptr = inp.get();
    parent->add_child(std::move(inp));

    s_row_y += ROW_H + ROW_GAP;
    return ptr;
}

static void add_section_header(ui::Control* parent, TextRenderer* tr,
    float panel_x, const std::string& title, Color col)
{
    s_row_y += 6.0f;
    auto hdr = std::make_unique<ui::Label>(tr, title);
    hdr->position  = {panel_x + 8.0f, s_row_y};
    hdr->font_size = UI_FONT;
    hdr->color     = col;
    parent->add_child(std::move(hdr));
    s_row_y += ROW_H + 2.0f;
}

static ui::Button* add_button(ui::Control* parent, TextRenderer* tr,
    float panel_x, const std::string& label, float width,
    Color normal, Color hover, Color pressed,
    std::function<void()> on_press)
{
    auto btn = std::make_unique<ui::Button>(tr, label);
    btn->position      = {panel_x + 8.0f, s_row_y};
    btn->size          = {width, ROW_H + 2.0f};
    btn->font_size     = UI_FONT;
    btn->color_normal  = normal;
    btn->color_hover   = hover;
    btn->color_pressed = pressed;
    btn->on_pressed    = on_press;
    ui::Button* ptr    = btn.get();
    parent->add_child(std::move(btn));
    s_row_y += ROW_H + ROW_GAP + 4.0f;
    return ptr;
}

// ============================================================
// Main
// ============================================================
int main() {
    try {
        Window window(WIN_W, WIN_H, "NovaGFX — Particle Showcase");
        window.set_vsync(true);

        Renderer2D  renderer;
        TextRenderer text;

        Camera2D camera(static_cast<float>(WIN_W), static_cast<float>(WIN_H));
        camera.set_zoom(1.0f);
        camera.set_position({WIN_W / 2.0f, WIN_H / 2.0f});

        // ── Particle system ──────────────────────────────────────────────
        ParticleSystem ps(65536);

        auto presets = make_presets();
        static constexpr int PRESET_COUNT = 8;

        // Register one emitter per preset
        std::array<usize, PRESET_COUNT> emitter_ids{};
        for (int i = 0; i < PRESET_COUNT; ++i) {
            emitter_ids[i] = ps.add_emitter(presets[i].emitter);
            // All start inactive; we'll enable only the active one
            ps.get_emitter(emitter_ids[i]).active = false;
        }

        int   active_preset = 0;
        float emitter_x     = WIN_W * 0.38f;
        float emitter_y     = WIN_H * 0.55f;

        // Enable the first preset
        ps.get_emitter(emitter_ids[active_preset]).active = true;
        ps.get_emitter(emitter_ids[active_preset]).position = {emitter_x, emitter_y};

        // ── UI Root ──────────────────────────────────────────────────────
        auto root_ui = std::make_unique<ui::Control>();
        root_ui->size = {static_cast<f32>(WIN_W), static_cast<f32>(WIN_H)};

        // ── Panel ────────────────────────────────────────────────────────
        float panel_y  = 4.0f;
        float panel_h  = static_cast<float>(WIN_H) - 8.0f;

        auto panel_node = std::make_unique<Panel>();
        Panel* panel_ptr = panel_node.get(); // keep a ptr for dynamic updates
        
        s_row_y = 10.0f;   // reset row cursor (relative to panel)

        // Title label in panel
        {
            auto t = std::make_unique<ui::Label>(&text, "PARTICLE SHOWCASE");
            t->position  = {8.0f, s_row_y};
            t->font_size = 20.0f;
            t->color     = {0.7f, 0.6f, 1.0f, 1.0f};
            panel_node->add_child(std::move(t));
            s_row_y += 28.0f + ROW_GAP;
        }

        // Particle count label
        ui::Label* count_lbl = nullptr;
        {
            auto lbl = std::make_unique<ui::Label>(&text, "Alive: 0");
            lbl->position  = {8.0f, s_row_y};
            lbl->font_size = UI_FONT;
            lbl->color     = {0.6f, 1.0f, 0.6f, 1.0f};
            count_lbl = lbl.get();
            panel_node->add_child(std::move(lbl));
            s_row_y += ROW_H + ROW_GAP;
        }

        // Active preset label
        ui::Label* preset_lbl = nullptr;
        {
            auto lbl = std::make_unique<ui::Label>(&text, "Effect: " + presets[active_preset].name);
            lbl->position  = {8.0f, s_row_y};
            lbl->font_size = UI_FONT;
            lbl->color     = {1.0f, 0.85f, 0.4f, 1.0f};
            preset_lbl = lbl.get();
            panel_node->add_child(std::move(lbl));
            s_row_y += ROW_H + ROW_GAP;
        }

        std::vector<FieldMapping> mappings;
        std::vector<ui::TextInput*> all_inputs;

        auto update_ui_from_current = [&]() {
            auto& em = ps.get_emitter(emitter_ids[active_preset]);
            for (auto& m : mappings) {
                m.input->set_float(m.getter(em), 2);
            }
            if (preset_lbl) preset_lbl->text = "Effect: " + presets[active_preset].name;
        };

        // ── Preset selector buttons ───────────────────────────────────────
        {
            auto hdr = std::make_unique<ui::Label>(&text, "Select Effect  [1-8]");
            hdr->position  = {8.0f, s_row_y};
            hdr->font_size = UI_FONT;
            hdr->color     = {0.5f, 0.8f, 1.0f, 1.0f};
            panel_node->add_child(std::move(hdr));
            s_row_y += ROW_H + 2.0f;
        }

        const float BTN_SEL_W = (PANEL_W - 20.0f) / 4.0f - 3.0f;
        const float BTN_SEL_H = 24.0f;
        for (int i = 0; i < PRESET_COUNT; ++i) {
            int row = i / 4, col = i % 4;
            float bx = 8.0f + col * (BTN_SEL_W + 3.0f);
            float by = s_row_y + row * (BTN_SEL_H + 3.0f);

            auto btn = std::make_unique<ui::Button>(&text, std::to_string(i + 1) + ":" + presets[i].name.substr(0, 3));
            btn->position      = {bx, by};
            btn->size          = {BTN_SEL_W, BTN_SEL_H};
            btn->font_size     = 13.0f;
            btn->color_normal  = {0.15f, 0.15f, 0.25f, 1.0f};
            btn->color_hover   = {0.25f, 0.25f, 0.45f, 1.0f};
            btn->color_pressed = {0.1f,  0.1f,  0.18f, 1.0f};

            btn->on_pressed = [&ps, &emitter_ids, &active_preset, i, update_ui_from_current]() {
                ps.get_emitter(emitter_ids[active_preset]).active = false;
                active_preset = i;
                ps.get_emitter(emitter_ids[active_preset]).active = true;
                update_ui_from_current();
            };
            panel_node->add_child(std::move(btn));
        }
        s_row_y += BTN_SEL_H * 2 + 3.0f + ROW_GAP + 4.0f;

        // ── Burst button ──────────────────────────────────────────────────
        add_button(panel_node.get(), &text, 0.0f,
            "BURST  [SPACE]", PANEL_W - 20.0f,
            {0.25f, 0.12f, 0.45f, 1.0f},
            {0.4f,  0.2f,  0.65f, 1.0f},
            {0.15f, 0.06f, 0.3f,  1.0f},
            [&ps, &emitter_ids, &active_preset]() {
                ps.emit(emitter_ids[active_preset], ps.get_emitter(emitter_ids[active_preset]).burst_count);
            });

        // ── Tweakable parameters ──────────────────────────────────────────
        add_section_header(panel_node.get(), &text, 0.0f,
            "-- EMITTER PARAMS --", {0.55f, 0.85f, 1.0f, 1.0f});

        auto add_ep = [&](const std::string& name, float step,
            auto field_ref,
            auto field_val)
        {
            auto& initial_emitter = ps.get_emitter(emitter_ids[active_preset]);
            auto* input = add_float_row(panel_node.get(), &text, 0.0f, name, field_val(initial_emitter), step,
                [&ps, &emitter_ids, &active_preset, field_ref](float v) {
                    field_ref(ps.get_emitter(emitter_ids[active_preset])) = v;
                });
            mappings.push_back({input, field_val});
            all_inputs.push_back(input);
        };

        add_ep("Rate (p/s)",    5.0f,  [](ParticleEmitter& e) -> float& { return e.rate; }, [](const ParticleEmitter& e) { return e.rate; });
        add_ep("Spread",        0.05f, [](ParticleEmitter& e) -> float& { return e.spread_angle; }, [](const ParticleEmitter& e) { return e.spread_angle; });
        add_ep("Vel Min",       10.0f, [](ParticleEmitter& e) -> float& { return e.velocity_min; }, [](const ParticleEmitter& e) { return e.velocity_min; });
        add_ep("Vel Max",       10.0f, [](ParticleEmitter& e) -> float& { return e.velocity_max; }, [](const ParticleEmitter& e) { return e.velocity_max; });
        add_ep("Life Min",      0.1f,  [](ParticleEmitter& e) -> float& { return e.lifetime_min; }, [](const ParticleEmitter& e) { return e.lifetime_min; });
        add_ep("Life Max",      0.1f,  [](ParticleEmitter& e) -> float& { return e.lifetime_max; }, [](const ParticleEmitter& e) { return e.lifetime_max; });
        add_ep("Size Start",    1.0f,  [](ParticleEmitter& e) -> float& { return e.size_start; }, [](const ParticleEmitter& e) { return e.size_start; });
        add_ep("Size End",      1.0f,  [](ParticleEmitter& e) -> float& { return e.size_end; }, [](const ParticleEmitter& e) { return e.size_end; });
        add_ep("Gravity",       0.1f,  [](ParticleEmitter& e) -> float& { return e.gravity_scale; }, [](const ParticleEmitter& e) { return e.gravity_scale; });
        add_ep("Drag",          0.1f,  [](ParticleEmitter& e) -> float& { return e.drag; }, [](const ParticleEmitter& e) { return e.drag; });
        
        // Burst Count
        {
            auto lbl = std::make_unique<ui::Label>(&text, "Burst Count");
            lbl->position  = {8.0f, s_row_y + 6.0f};
            lbl->font_size = UI_FONT;
            lbl->color     = {0.85f, 0.85f, 0.95f, 1.0f};
            panel_node->add_child(std::move(lbl));

            auto inp = std::make_unique<ui::TextInput>(&text, "", ui::InputMode::INT);
            inp->set_int(ps.get_emitter(emitter_ids[active_preset]).burst_count);
            inp->position  = {LABEL_W + 8.0f, s_row_y};
            inp->size      = {INPUT_W, ROW_H};
            inp->font_size = UI_FONT;
            inp->step      = 5.0f;
            inp->color_bg_normal  = {0.05f, 0.06f, 0.12f, 1.0f};
            inp->color_bg_focused = {0.12f, 0.14f, 0.28f, 1.0f};
            inp->color_border     = {0.35f, 0.35f, 0.65f, 1.0f};
            inp->on_committed = [&ps, &emitter_ids, &active_preset](const std::string& s) {
                try { ps.get_emitter(emitter_ids[active_preset]).burst_count = std::stoi(s); } catch (...) {}
            };
            auto* pi = inp.get();
            panel_node->add_child(std::move(inp));
            
            all_inputs.push_back(pi);
            mappings.push_back({pi, [](const ParticleEmitter& e) { return static_cast<float>(e.burst_count); }});
            s_row_y += ROW_H + ROW_GAP;
        }

        // ── Reset button ─────────────────────────────────────────────────
        s_row_y += 6.0f;
        add_button(panel_node.get(), &text, 0.0f,
            "Reset to Preset", PANEL_W - 20.0f,
            {0.15f, 0.35f, 0.15f, 1.0f},
            {0.2f,  0.55f, 0.2f,  1.0f},
            {0.08f, 0.2f,  0.08f, 1.0f},
            [&ps, &emitter_ids, &active_preset, &presets, update_ui_from_current]() {
                Vector2f pos = ps.get_emitter(emitter_ids[active_preset]).position;
                ps.get_emitter(emitter_ids[active_preset]) = presets[active_preset].emitter;
                ps.get_emitter(emitter_ids[active_preset]).position = pos;
                ps.get_emitter(emitter_ids[active_preset]).active   = true;
                update_ui_from_current();
            });

        // ── Instructions ──────────────────────────────────────────────────
        {
            auto lbl = std::make_unique<ui::Label>(&text, "LMB: move | Scroll: zoom");
            lbl->position  = {8.0f, s_row_y};
            lbl->font_size = 14.0f;
            lbl->color     = {0.5f, 0.5f, 0.6f, 1.0f};
            panel_node->add_child(std::move(lbl));
            s_row_y += 20.0f;
            auto lbl2 = std::make_unique<ui::Label>(&text, "ESC: quit");
            lbl2->position  = {8.0f, s_row_y};
            lbl2->font_size = 14.0f;
            lbl2->color     = {0.5f, 0.5f, 0.6f, 1.0f};
            panel_node->add_child(std::move(lbl2));
        }

        root_ui->add_child(std::move(panel_node));

        auto last_time = std::chrono::high_resolution_clock::now();
        float auto_burst_timer = 0.0f;

        int frame_count = 0;
        float fps_timer = 0.0f;
        std::string fps_text  = "FPS: 0";
        std::string stat_text = "";
        float frame_time_ms   = 0.0f;   // smoothed last-frame ms

        while (!window.should_close()) {
            auto now   = std::chrono::high_resolution_clock::now();
            f32  delta = std::chrono::duration<f32>(now - last_time).count();
            last_time  = now;
            if (delta > 0.1f) delta = 0.1f;

            Input::update();
            window.poll_events();

            if (Input::is_key_pressed(Key::ESCAPE)) window.close();

            // ── Check focus ───────────────────────────────────────────
            bool any_focus = false;
            for (auto* in : all_inputs) if (in->is_focused) any_focus = true;

            // ── Scroll zoom ───────────────────────────────────────────
            Vector2f scroll = Input::get_scroll_delta();
            if (std::abs(scroll.y) > 0.01f) {
                float zoom = camera.get_zoom();
                zoom *= (1.0f + scroll.y * 0.1f);
                camera.set_zoom(std::clamp(zoom, 0.1f, 10.0f));
            }

            // ── Hotkeys ───────────────────────────────────────────────
            if (!any_focus) {
                Key num_keys[8] = { Key::NUM_1, Key::NUM_2, Key::NUM_3, Key::NUM_4, Key::NUM_5, Key::NUM_6, Key::NUM_7, Key::NUM_8 };
                Key kp_keys[8]  = { Key::KP_1, Key::KP_2, Key::KP_3, Key::KP_4, Key::KP_5, Key::KP_6, Key::KP_7, Key::KP_8 };
                for (int i = 0; i < PRESET_COUNT; ++i) {
                    if (Input::is_key_pressed(num_keys[i]) || Input::is_key_pressed(kp_keys[i])) {
                        ps.get_emitter(emitter_ids[active_preset]).active = false;
                        active_preset = i;
                        ps.get_emitter(emitter_ids[active_preset]).active = true;
                        ps.get_emitter(emitter_ids[active_preset]).position = {emitter_x, emitter_y};
                        update_ui_from_current();
                    }
                }

                if (Input::is_key_pressed(Key::SPACE) || Input::is_key_pressed(Key::KP_ADD)) {
                    ps.emit(emitter_ids[active_preset], ps.get_emitter(emitter_ids[active_preset]).burst_count);
                }
            }

            // ── Update UI dynamic layout ─────────────────────────────
            Vector2i wsize = window.get_size();
            root_ui->size = {(f32)wsize.x, (f32)wsize.y};
            float current_panel_x = std::max(0.0f, (f32)wsize.x - PANEL_W - 6.0f);
            panel_ptr->position = {current_panel_x, 4.0f};
            panel_ptr->size     = {PANEL_W, (f32)wsize.y - 8.0f};

            // ── Reposition emitter (Screen-to-World) ──────────────────
            Vector2f mouse_pos = Input::get_mouse_position();
            bool mouse_down     = Input::is_mouse_down(MouseButton::LEFT);

            if (mouse_down && mouse_pos.x < current_panel_x) {
                // Manual screen-to-world conversion:
                // (MousePos - ViewportHalf) / Zoom + CameraPos
                Vector2f viewport_half = {(f32)wsize.x * 0.5f, (f32)wsize.y * 0.5f};
                Vector2f world_mouse = (mouse_pos - viewport_half) / camera.get_zoom() + camera.get_position();
                
                emitter_x = world_mouse.x;
                emitter_y = world_mouse.y;
                ps.get_emitter(emitter_ids[active_preset]).position = {emitter_x, emitter_y};
            }

            // ── Auto burst ────────────────────────────────────────────
            auto& em = ps.get_emitter(emitter_ids[active_preset]);
            if (em.mode == EmitterMode::BURST) {
                auto_burst_timer += delta;
                if (auto_burst_timer > 1.5f) {
                    ps.emit(emitter_ids[active_preset], em.burst_count);
                    auto_burst_timer = 0.0f;
                }
            }

            // ── Update FPS / stats tracker ─────────────────────────────
            frame_time_ms = frame_time_ms * 0.85f + (delta * 1000.0f) * 0.15f; // EMA
            frame_count++;
            fps_timer += delta;
            if (fps_timer >= 1.0f) {
                fps_text = "FPS: " + std::to_string(frame_count);
                // Build stats string (updated once/sec to stay readable)
                auto fmt1 = [](float v, int dec) {
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "%.*f", dec, v);
                    return std::string(buf);
                };
                stat_text =
                    "Frame time : " + fmt1(frame_time_ms, 2) + " ms\n"
                    "Particles  : " + std::to_string(ps.alive_count()) + " / 65536\n"
                    "Zoom       : " + fmt1(camera.get_zoom(), 2) + "x\n"
                    "Emitter    : (" + fmt1(emitter_x, 0) + ", " + fmt1(emitter_y, 0) + ")\n"
                    "Effect     : " + presets[active_preset].name;
                frame_count = 0;
                fps_timer -= 1.0f;
            }

            // ── UI tree update ────────────────────────────────────────
            root_ui->update_ui_tree(delta, mouse_pos, Input::is_mouse_pressed(MouseButton::LEFT), Input::is_mouse_released(MouseButton::LEFT));
            ps.update(delta);

            if (count_lbl) count_lbl->text = "Alive: " + std::to_string(ps.alive_count());

            // ── Render ────────────────────────────────────────────────
            wsize = window.get_size();
            glViewport(0, 0, wsize.x, wsize.y);
            glClearColor(0.04f, 0.04f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.init_camera(camera.get_position(), camera.get_zoom(), { (f32)wsize.x, (f32)wsize.y });
            renderer.begin();
            {
                Color grid_col{0.12f, 0.12f, 0.22f, 1.0f};
                f32 cz = camera.get_zoom();
                // Draw crosshairs at emitter pos, scaled to cover the viewable area
                float vw = (f32)wsize.x / cz;
                float vh = (f32)wsize.y / cz;
                renderer.draw_rect({emitter_x - vw, emitter_y - 0.5f / cz}, {vw * 2.0f, 1.0f / cz}, grid_col);
                renderer.draw_rect({emitter_x - 0.5f / cz, emitter_y - vh}, {1.0f / cz, vh * 2.0f}, grid_col);
                
                renderer.draw_circle_outline({emitter_x, emitter_y}, 14.0f / cz, {0.4f, 0.4f, 0.7f, 0.7f}, 1.5f / cz);
                renderer.draw_circle({emitter_x, emitter_y}, 3.0f / cz, {0.7f, 0.7f, 1.0f, 1.0f});
            }
            ps.draw(renderer);
            renderer.end();

            renderer.init_camera({wsize.x / 2.0f, wsize.y / 2.0f}, 1.0f, {(f32)wsize.x, (f32)wsize.y});
            renderer.begin();
            
            // ── Stats overlay (top-left) ──────────────────────────────
            // Row positions
            float sx = 10.0f;
            float sy = 10.0f;
            float stat_font = 18.0f;
            float stat_line = stat_font + 5.0f;

            // Semi-transparent backdrop
            renderer.draw_rect({sx - 6.0f, sy - 4.0f}, {290.0f, stat_line * 6.0f + 8.0f},
                Color(0.0f, 0.0f, 0.0f, 0.50f));

            // FPS in bright green
            text.draw_text(renderer, fps_text,     {sx, sy},                stat_font + 4.0f, Color(0.3f, 1.0f, 0.4f, 1.0f));
            // Frame time
            text.draw_text(renderer, "Frame: " + [&]{ char b[16]; std::snprintf(b,16,"%.2f ms",frame_time_ms); return std::string(b); }(),
                {sx, sy + stat_line},     stat_font, Color(0.85f, 0.85f, 0.55f, 1.0f));
            // Particles
            text.draw_text(renderer, "Particles: " + std::to_string(ps.alive_count()) + " / 65536",
                {sx, sy + stat_line*2},   stat_font, Color(0.6f, 0.85f, 1.0f, 1.0f));
            // Effect
            text.draw_text(renderer, "Effect: " + presets[active_preset].name,
                {sx, sy + stat_line*3},   stat_font, Color(1.0f, 0.85f, 0.4f, 1.0f));
            // Zoom
            text.draw_text(renderer, [&]{ char b[32]; std::snprintf(b,32,"Zoom: %.2fx",camera.get_zoom()); return std::string(b); }(),
                {sx, sy + stat_line*4},   stat_font, Color(0.75f, 0.75f, 0.95f, 1.0f));
            // Emitter pos
            text.draw_text(renderer, [&]{ char b[48]; std::snprintf(b,48,"Emitter: (%.0f, %.0f)",emitter_x,emitter_y); return std::string(b); }(),
                {sx, sy + stat_line*5},   stat_font, Color(0.7f, 0.7f, 0.7f, 1.0f));

            root_ui->draw_tree(renderer);
            renderer.end();

            window.swap_buffers();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
