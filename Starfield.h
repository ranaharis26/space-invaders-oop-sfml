#pragma once
#include <SFML/Graphics.hpp>

struct Star {
    sf::CircleShape shape;
    float speed;
};

class Starfield {
public:
    Starfield(float screenW, float screenH, int count = 150);
    ~Starfield() { delete[] stars; }
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    float screenW, screenH;
    Star* stars;
    int starCount;
};
