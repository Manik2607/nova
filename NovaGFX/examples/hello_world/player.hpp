#pragma once
#include <nova/nova.hpp>
#include <iostream>
#include <chrono>
using namespace nova;
class Player{
private:
    float player_speed = 300; 
    Vector2f player_pos;
    Texture2D player_texture;

public:
    Player();
    void update(float delta);
    void draw(Renderer2D* renderer);

};