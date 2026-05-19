#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>


class Asteroid : public Entity {
public:
    Asteroid(float x, float y, float size, float speed);
    virtual ~Asteroid() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override {}

    float getRadius() const { return radius; }

private:
    float radius;
    float rotationAngle;
    float rotationSpeed;
    sf::ConvexShape shape;
    void buildShape(int seed);
};
