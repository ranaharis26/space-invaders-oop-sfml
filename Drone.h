#pragma once
#include "Enemy.h"

class Drone : public Enemy {
public:
    Drone(float x, float y, float speedScale = 1.0f, float fireRateScale = 1.0f);
    virtual ~Drone() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    Bullet* shoot() override;

private:
    sf::ConvexShape bodyShape;
    sf::RectangleShape wingL, wingR;
    sf::CircleShape cockpit;
    void buildSprite();
};
