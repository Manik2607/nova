#pragma once
#include <nova/nova.hpp>
#include "nova/renderer/text_renderer.hpp"
#include "drift_scorer.hpp"
using namespace nova;

/// Draws on-screen HUD elements in screen-space using text rendering.
class HUD {
public:
    void draw(Renderer2D& renderer, TextRenderer& text,
              Vector2f viewport, f32 speed, const DriftScorer& scorer, f32 fps);
};
