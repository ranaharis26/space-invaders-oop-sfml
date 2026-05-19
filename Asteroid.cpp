#include "Asteroid.h"
#include <cstdlib>
#include <cmath>

Asteroid::Asteroid(float x, float y, float size, float speed)
    : Entity(x, y, size*2.f, size*2.f), radius(size),
      rotationAngle(0), rotationSpeed((float)(std::rand()%60+20))
{
    velocity = {0.f, speed};
    buildShape(std::rand());
}

void Asteroid::buildShape(int seed) {
    const int pts = 10;
    shape.setPointCount(pts);
    std::srand(seed);
    for (int i = 0; i < pts; i++) {
        float angle = (float)i / pts * 2.f * 3.14159f;
        float r = radius * (0.7f + 0.3f * ((std::rand() % 100) / 100.f));
        shape.setPoint(i, {r * std::cos(angle), r * std::sin(angle)});
    }
    shape.setFillColor(sf::Color(100, 90, 80));
    shape.setOutlineColor(sf::Color(170, 155, 140));
    shape.setOutlineThickness(2.f);
}

void Asteroid::update(float dt) {
    Entity::update(dt);
    rotationAngle += rotationSpeed * dt;
    if (position.y > 760.f) {
        position.y = -radius;
        position.x = (float)(std::rand() % 860 + 20);
    }
}

void Asteroid::draw(sf::RenderWindow& window) {
    shape.setPosition(position);
    shape.setRotation(sf::degrees(rotationAngle));
    window.draw(shape);
}
