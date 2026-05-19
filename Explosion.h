#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float life;
    float maxLife;
    sf::Color color;
    float size;
};

class Explosion {
public:
    Explosion(float x, float y, sf::Color color, int count = 20, float radius = 80.f);
    void update(float dt);
    void draw(sf::RenderWindow& window);
    bool isDone() const { return particles.empty(); }

private:
    std::vector<Particle> particles;
};
