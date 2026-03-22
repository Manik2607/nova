#include "player.hpp"

using namespace nova;


Player::Player() : 
    player_pos(10.0f, 10.0f), 
    player_texture("anime_girl.png") 
    {

    }

void Player::update(float delta){
        if (Input::is_key_down(Key::W)) player_pos.y -= player_speed * delta;
        if (Input::is_key_down(Key::S)) player_pos.y += player_speed * delta;
        if (Input::is_key_down(Key::A)) player_pos.x -= player_speed * delta;
        if (Input::is_key_down(Key::D)) player_pos.x += player_speed * delta;
        
    }
    void Player::draw(Renderer2D *renderer){
    f32 angle = (player_pos - Input::get_mouse_position()).angle();
    renderer->draw_sprite(player_texture, player_pos, Vector2f(100, 100), Color::WHITE(), angle, {0.5f, 0.5f});
}
