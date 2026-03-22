#pragma once
#include <nova/nova.hpp>
#include <iostream>
#include <vector>
#include <chrono>
class Player{
public:
    Vector2f player_pos(0,0);
    Texture2D player_texture("anime_girl.png");
    Player(){
        
    }
    void update(float delta);
    void draw(Renderer2D* renderer);

};