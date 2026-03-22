#include "player.hpp"

using namespace nova;

void Player::update(float delta){
        if (Input::is_key_down(Key::W)) player_pos.y -= player_speed * delta;
        if (Input::is_key_down(Key::S)) player_pos.y += player_speed * delta;
        if (Input::is_key_down(Key::A)) player_pos.x -= player_speed * delta;
        if (Input::is_key_down(Key::D)) player_pos.x += player_speed * delta;
}
void Player::draw(Renderer2D *renderer){
    renderer->draw_sprite(player_texture, player_pos, Vector2f(300, 300), Color::WHITE(), 0.0f, {0.5f, 0.5f});
}
